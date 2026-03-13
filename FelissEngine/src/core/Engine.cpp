#include "core/Engine.h"
#include "platform/Window.h"
#include "renderer/RenderPipeline.h"
#include "ecs/World.h"
#include "scripting/ScriptEngine.h"
#include "physics/PhysicsWorld.h"
#include "audio/AudioEngine.h"
#include "editor/ExtensionManager.h"

#ifdef FELISS_EDITOR_BUILD
#  include "editor/EditorApp.h"
#endif

namespace Feliss {

Engine::Engine(const EngineConfig& cfg) : m_cfg(cfg) {
    Logger::get().setMinLevel(LogLevel::Debug);
    if (!cfg.logFile.empty()) Logger::get().setOutputFile(cfg.logFile);
    FLS_INFOF("Engine", "Feliss Engine v" << getVersion().toString());
}

Engine::~Engine() { shutdown(); }

bool Engine::init() {
    FLS_INFO("Engine", "Initialising subsystems...");

    // Window
    WindowDesc wd;
    wd.title      = m_cfg.title;
    wd.width      = m_cfg.windowWidth;
    wd.height     = m_cfg.windowHeight;
    wd.fullscreen = m_cfg.fullscreen;
    wd.vsync      = m_cfg.vsync;
    m_window = MakeScope<Window>(wd);
    if (!m_window->init()) { FLS_ERROR("Engine","Window init failed"); return false; }

    m_window->setResizeCallback([this](int w, int h){
        if (m_renderer) m_renderer->onResize(w, h);
        m_events.emit(WindowResizeEvent{w, h});
    });
    m_window->setCloseCallback([this](){
        m_events.emit(WindowCloseEvent{});
        m_running = false;
    });

    // Renderer
    RenderPipelineDesc rd;
    rd.api    = m_cfg.renderAPI;
    rd.width  = m_cfg.windowWidth;
    rd.height = m_cfg.windowHeight;
    rd.vsync  = m_cfg.vsync;
    m_renderer = MakeScope<RenderPipeline>(rd);
    if (!m_renderer->init(*m_window)) { FLS_ERROR("Engine","Renderer init failed"); return false; }

#ifdef FELISS_EDITOR_BUILD
    m_renderer->backend().imguiInit();
#endif

    // ECS World
    m_world = MakeScope<World>();
    m_world->onEntityCreated   = [this](EntityID id){ m_events.emitDeferred(EntityCreatedEvent{id}); };
    m_world->onEntityDestroyed = [this](EntityID id){ m_events.emitDeferred(EntityDestroyedEvent{id}); };

    // Physics
    m_physics = MakeScope<PhysicsWorld>();
    m_physics->init();

    // Audio
    m_audio = MakeScope<AudioEngine>();
    m_audio->init();

    // Scripting
    ScriptEngineConfig sc;
    sc.enableLua    = m_cfg.enableLua;
    sc.enableCSharp = m_cfg.enableCSharp;
    m_scripts = MakeScope<ScriptEngine>();
    if (!m_scripts->init(sc)) { FLS_WARN("Engine","Script engine init failed (non-fatal)"); }
    if (m_cfg.enableLua && m_scripts->lua()) m_scripts->lua()->bindAPI(*this);
    if (m_cfg.enableCSharp && m_scripts->csharp()) m_scripts->csharp()->bindAPI(*this);

    // Extensions
    m_extensions = MakeScope<ExtensionManager>(*this);
    m_extensions->scanDirectory(m_cfg.projectPath + "/extensions");

    m_initialized = true;
    FLS_INFO("Engine","All subsystems ready");
    return true;
}

void Engine::run() {
    if (!m_initialized) { FLS_ERROR("Engine","run() called before init()"); return; }
    m_running = true;
    FLS_INFO("Engine","Entering main loop");

    while (m_running && !m_window->shouldClose()) {
        processFrame();
    }

    FLS_INFO("Engine","Main loop exited");
    m_events.emit(EngineShutdownEvent{});
}

void Engine::processFrame() {
    m_dt.tick();
    f64 dt = m_dt.deltaTime();

    m_window->pollEvents();
    m_events.flush();

    // Fixed-timestep physics
    m_fixedAccum += dt;
    while (m_fixedAccum >= m_cfg.fixedTimestep) {
        m_physics->step(m_cfg.fixedTimestep);
        m_fixedAccum -= m_cfg.fixedTimestep;
    }

    // Game update
    m_world->update(static_cast<f32>(dt));
    m_scripts->update(static_cast<f32>(dt));
    m_extensions->update(static_cast<f32>(dt));
    m_audio->update(static_cast<f32>(dt));
    if (m_updateCb) m_updateCb(static_cast<f32>(dt));

    // Render
    m_renderer->beginFrame();
    m_world->render(*m_renderer);
    if (m_renderCb) m_renderCb();

#ifdef FELISS_EDITOR_BUILD
    m_renderer->beginImGui();
    m_extensions->onImGui();
    m_renderer->endImGui();
#endif

    m_renderer->endFrame();
}

void Engine::stop()     { m_running = false; }

void Engine::shutdown() {
    if (!m_initialized) return;
    FLS_INFO("Engine","Shutting down...");
    if (m_extensions) m_extensions->unloadAll();
    if (m_scripts)    m_scripts->shutdown();
    if (m_audio)      m_audio->shutdown();
    if (m_physics)    m_physics->shutdown();
    if (m_world)      m_world->clear();
#ifdef FELISS_EDITOR_BUILD
    if (m_renderer)   m_renderer->backend().imguiShutdown();
#endif
    if (m_renderer)   m_renderer->shutdown();
    if (m_window)     m_window->shutdown();
    m_initialized = false;
    FLS_INFO("Engine","Shutdown complete");
}

} // namespace Feliss
