#pragma once
#include "feliss/Types.h"
#include <typeindex>
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <any>

namespace Feliss {

// =====================================================================
// Built-in engine events
// =====================================================================
struct WindowResizeEvent   { int width=0, height=0; };
struct WindowCloseEvent    {};
struct WindowFocusEvent    { bool focused=false; };
struct KeyPressedEvent     { KeyCode key=KeyCode::Unknown; bool repeat=false; int mods=0; };
struct KeyReleasedEvent    { KeyCode key=KeyCode::Unknown; };
struct KeyTypedEvent       { uint32_t codepoint=0; };
struct MouseMovedEvent     { float x=0,y=0,dx=0,dy=0; };
struct MouseScrolledEvent  { float xOff=0, yOff=0; };
struct MouseButtonDownEvent{ MouseButton btn=MouseButton::Left; float x=0,y=0; };
struct MouseButtonUpEvent  { MouseButton btn=MouseButton::Left; float x=0,y=0; };
struct EntityCreatedEvent  { EntityID id=NULL_ENTITY; std::string name; };
struct EntityDestroyedEvent{ EntityID id=NULL_ENTITY; };
struct SceneLoadedEvent    { std::string path; };
struct SceneUnloadedEvent  { std::string name; };
struct ExtensionLoadedEvent  { std::string id; };
struct ExtensionUnloadedEvent{ std::string id; };
struct EngineShutdownEvent {};

// =====================================================================
// EventSystem — type-safe, thread-compatible event bus
// =====================================================================
class EventSystem {
public:
    EventSystem() = default;
    ~EventSystem() = default;
    EventSystem(const EventSystem&)            = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    // Subscribe (immediate): returns listener ID for later unsubscribe
    template<typename E>
    ListenerID subscribe(std::function<void(const E&)> fn) {
        auto idx = std::type_index(typeid(E));
        ListenerID lid = ++m_nextID;
        std::lock_guard<std::mutex> lk(m_mutex);
        m_listeners[idx].push_back({
            lid,
            [f = std::move(fn)](const std::any& a){ f(std::any_cast<const E&>(a)); }
        });
        return lid;
    }

    void unsubscribe(ListenerID lid);

    // Emit immediately (synchronous)
    template<typename E>
    void emit(const E& ev) {
        auto idx = std::type_index(typeid(E));
        std::vector<Entry> copy;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto it = m_listeners.find(idx);
            if (it == m_listeners.end()) return;
            copy = it->second;
        }
        std::any a(ev);
        for (auto& e : copy) e.fn(a);
    }

    // Emit deferred (collected, dispatched in flush())
    template<typename E>
    void emitDeferred(const E& ev) {
        auto idx = std::type_index(typeid(E));
        std::lock_guard<std::mutex> lk(m_qMutex);
        m_queue.push({idx, std::any(ev)});
    }

    // Call once per frame to dispatch deferred events
    void flush();
    void clear();
    size_t listenerCount() const;

private:
    struct Entry { ListenerID lid; std::function<void(const std::any&)> fn; };
    struct Queued{ std::type_index idx; std::any ev; };

    std::unordered_map<std::type_index, std::vector<Entry>> m_listeners;
    std::queue<Queued>  m_queue;
    mutable std::mutex  m_mutex;
    mutable std::mutex  m_qMutex;
    ListenerID          m_nextID = 0;
};

} // namespace Feliss
