#pragma once
#include "ecs/Component.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <typeindex>
#include <string>

namespace Feliss {
class RenderPipeline;

// =====================================================================
// World — owns all entities and their components
// =====================================================================
struct EntityRecord {
    EntityID    id     = NULL_ENTITY;
    std::string name;
    std::string tag;
    bool        active = true;
    EntityID    parent = NULL_ENTITY;
    std::vector<EntityID> children;
    std::unordered_map<std::type_index, std::unique_ptr<ComponentBase>> comps;
};

class World {
public:
    World();
    ~World();
    World(const World&)            = delete;
    World& operator=(const World&) = delete;

    // ---- Entity ----
    EntityID      createEntity(const std::string& name = "Entity");
    void          destroyEntity(EntityID id);
    bool          isValid(EntityID id)            const;
    void          setActive(EntityID id, bool v);
    bool          isActive(EntityID id)           const;
    void          setName(EntityID id, const std::string& n);
    std::string   getName(EntityID id)            const;
    void          setTag(EntityID id, const std::string& t);
    std::string   getTag(EntityID id)             const;

    // ---- Hierarchy ----
    void          setParent(EntityID child, EntityID parent);
    EntityID      getParent(EntityID child)        const;
    const std::vector<EntityID>& getChildren(EntityID id) const;
    std::vector<EntityID>        getRoots()        const;

    // ---- Components ----
    template<typename T, typename... Args>
    T& addComponent(EntityID id, Args&&... args) {
        FLS_ASSERT(isValid(id), "addComponent: invalid entity");
        auto& rec = m_entities.at(id);
        auto  key = std::type_index(typeid(T));
        auto  comp= std::make_unique<T>(std::forward<Args>(args)...);
        T*    ptr = comp.get();
        rec.comps[key] = std::move(comp);
        return *ptr;
    }
    template<typename T>
    T* getComponent(EntityID id) {
        auto it = m_entities.find(id);
        if (it == m_entities.end()) return nullptr;
        auto& comps = it->second.comps;
        auto  ci    = comps.find(std::type_index(typeid(T)));
        if (ci == comps.end()) return nullptr;
        return static_cast<T*>(ci->second.get());
    }
    template<typename T>
    const T* getComponent(EntityID id) const {
        auto it = m_entities.find(id);
        if (it == m_entities.end()) return nullptr;
        auto& comps = it->second.comps;
        auto  ci    = comps.find(std::type_index(typeid(T)));
        if (ci == comps.end()) return nullptr;
        return static_cast<const T*>(ci->second.get());
    }
    template<typename T>
    bool hasComponent(EntityID id) const {
        auto it = m_entities.find(id);
        if (it == m_entities.end()) return false;
        return it->second.comps.count(std::type_index(typeid(T))) > 0;
    }
    template<typename T>
    void removeComponent(EntityID id) {
        auto it = m_entities.find(id);
        if (it == m_entities.end()) return;
        it->second.comps.erase(std::type_index(typeid(T)));
    }

    // ---- Query ----
    template<typename T>
    void each(std::function<void(EntityID, T&)> fn) {
        auto key = std::type_index(typeid(T));
        for (auto& [id, rec] : m_entities) {
            if (!rec.active) continue;
            auto ci = rec.comps.find(key);
            if (ci != rec.comps.end())
                fn(id, *static_cast<T*>(ci->second.get()));
        }
    }
    template<typename T1, typename T2>
    void each(std::function<void(EntityID, T1&, T2&)> fn) {
        auto k1 = std::type_index(typeid(T1));
        auto k2 = std::type_index(typeid(T2));
        for (auto& [id, rec] : m_entities) {
            if (!rec.active) continue;
            auto it1 = rec.comps.find(k1);
            auto it2 = rec.comps.find(k2);
            if (it1 != rec.comps.end() && it2 != rec.comps.end())
                fn(id, *static_cast<T1*>(it1->second.get()),
                       *static_cast<T2*>(it2->second.get()));
        }
    }

    // ---- Find ----
    EntityID              findByName(const std::string& n) const;
    std::vector<EntityID> findByTag(const std::string& t)  const;

    // ---- Lifecycle ----
    void update(f32 dt);
    void render(RenderPipeline& rp);
    void clear();

    // ---- Serialization ----
    bool saveToFile(const std::string& path)  const;
    bool loadFromFile(const std::string& path);

    size_t entityCount() const { return m_entities.size(); }
    const std::unordered_map<EntityID, EntityRecord>& all() const { return m_entities; }

    // ---- Callbacks ----
    std::function<void(EntityID)> onEntityCreated;
    std::function<void(EntityID)> onEntityDestroyed;

private:
    EntityID generateID() { return m_nextID++; }
    void     detachFromParent(EntityID id);

    std::unordered_map<EntityID, EntityRecord> m_entities;
    EntityID m_nextID = 1;
    static const std::vector<EntityID> s_noChildren;
};

} // namespace Feliss
