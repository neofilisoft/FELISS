#pragma once
#include "feliss/Types.h"
#include "core/Logger.h"
#include "core/EventSystem.h"
#include "core/Timer.h"
#include <string>
#include <memory>
#include <functional>

namespace Feliss {

class Window;
class RenderPipeline;
class World;
class ScriptEngine;
class PhysicsWorld;
class AudioEngine;
class ExtensionManager;
class EditorApp;

// =====================================================================
// EngineConfig
// =====================================================================
struct EngineConfig {
    std::string title         = "Feliss Engine";
    int         windowWidth   = 1280;
    int         windowHeight  = 720;
    bool        fullscreen    = false;
    bool        vsync         = true;
    RenderAPI   renderAPI     = RenderAPI::OpenGL;
    EngineMode  mode          = EngineMode::Editor;
    std::string projectPath   = ".";
    std::string logFile       = "feliss.log";
    bool        enableLua     = true;
    bool        enableCSharp  = false;
    int         targetFPS     = 0;          // 0 = unlimited
    f32         fixedTimestep = 1.0f/60.0f;
};

// =====================================================================
// Engine — central hub, owns all subsystems
// =====================================================================
class Engine {
public:
    explicit Engine(const EngineConfig& cfg);
    ~Engine();
    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;

    bool init();
    void run();
    void stop();
    void shutdown();

    // Optional per-frame callbacks (for embedding)
    using UpdateCb = std::function<void(f32 dt)>;
    using RenderCb = std::function<void()>;
    void setUpdateCallback(UpdateCb cb)  { m_updateCb = std::move(cb); }
    void setRenderCallback(RenderCb cb)  { m_renderCb = std::move(cb); }

    // State
    bool      isRunning()     const { return m_running; }
    bool      isInitialized() const { return m_initialized; }
    EngineMode mode()         const { return m_cfg.mode; }
    const EngineConfig& config() const { return m_cfg; }

    // Timing
    f64 deltaTime()   const { return m_dt.deltaTime(); }
    f64 elapsedTime() const { return m_dt.totalTime(); }
    f32 fps()         const { return m_dt.fps(); }
    u64 frameCount()  const { return m_dt.frameCount(); }

    // Subsystems
    EventSystem&      events()     { return m_events; }
    Window&           window()     { return *m_window; }
    RenderPipeline&   renderer()   { return *m_renderer; }
    World&            world()      { return *m_world; }
    ScriptEngine&     scripts()    { return *m_scripts; }
    PhysicsWorld&     physics()    { return *m_physics; }
    AudioEngine&      audio()      { return *m_audio; }
    ExtensionManager& extensions() { return *m_extensions; }

    static Version getVersion() { return {0,1,0}; }

private:
    void processFrame();

    EngineConfig m_cfg;
    bool         m_running     = false;
    bool         m_initialized = false;
    DeltaTimer   m_dt;
    f64          m_fixedAccum  = 0.0;

    EventSystem                 m_events;
    Scope<Window>               m_window;
    Scope<RenderPipeline>       m_renderer;
    Scope<World>                m_world;
    Scope<ScriptEngine>         m_scripts;
    Scope<PhysicsWorld>         m_physics;
    Scope<AudioEngine>          m_audio;
    Scope<ExtensionManager>     m_extensions;

    UpdateCb m_updateCb;
    RenderCb m_renderCb;
};

} // namespace Feliss
