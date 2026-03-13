/*
 * feliss_api.cpp — Implementation of the stable C ABI (feliss_api.h)
 * Routes every fls_* call to the corresponding C++ subsystem.
 */
#define FELISS_EXPORT
#include "feliss/feliss_api.h"
#include "core/Engine.h"
#include "core/Logger.h"
#include "ecs/World.h"
#include "ecs/Component.h"
#include "platform/Window.h"

// Helper: reinterpret opaque handles
static Feliss::Engine*  E(FlsEngine e) { return static_cast<Feliss::Engine*>(e); }
static Feliss::World*   W(FlsScene  s) { return static_cast<Feliss::World*>(s); }

static thread_local std::string s_lastError;

extern "C" {

// ---- Version ----
FlsVersion fls_get_version(void) {
    auto v = Feliss::Engine::getVersion();
    return {v.major, v.minor, v.patch};
}
const char* fls_version_string(void) {
    static std::string vs = Feliss::Engine::getVersion().toString();
    return vs.c_str();
}

// ---- Error ----
const char* fls_status_string(FlsStatus s) {
    switch(s) {
        case FLS_OK:            return "OK";
        case FLS_ERR:           return "Error";
        case FLS_NOT_FOUND:     return "NotFound";
        case FLS_INVALID_ARG:   return "InvalidArg";
        case FLS_UNSUPPORTED:   return "Unsupported";
        case FLS_ALREADY_EXISTS:return "AlreadyExists";
        case FLS_OUT_OF_MEMORY: return "OutOfMemory";
        default:                return "Unknown";
    }
}
const char* fls_last_error(void) { return s_lastError.c_str(); }

// ---- Engine lifecycle ----
FlsEngine fls_engine_create(const FlsEngineDesc* d) {
    if (!d) return nullptr;
    Feliss::EngineConfig cfg;
    if (d->title)       cfg.title        = d->title;
    cfg.windowWidth  = d->windowW;
    cfg.windowHeight = d->windowH;
    cfg.fullscreen   = d->fullscreen;
    cfg.vsync        = d->vsync;
    cfg.renderAPI    = static_cast<Feliss::RenderAPI>(d->renderAPI);
    cfg.mode         = static_cast<Feliss::EngineMode>(d->mode);
    if (d->projectPath) cfg.projectPath  = d->projectPath;
    if (d->logFile)     cfg.logFile      = d->logFile;
    cfg.enableLua    = d->enableLua;
    cfg.enableCSharp = d->enableCSharp;
    cfg.targetFPS    = d->targetFPS;
    return new Feliss::Engine(cfg);
}
void fls_engine_destroy(FlsEngine e) { delete E(e); }
FlsStatus fls_engine_init(FlsEngine e) {
    if (!e) return FLS_INVALID_ARG;
    return E(e)->init() ? FLS_OK : FLS_ERR;
}
void fls_engine_run(FlsEngine e) { if (e) E(e)->run(); }
void fls_engine_stop(FlsEngine e) { if (e) E(e)->stop(); }
bool fls_engine_is_running(FlsEngine e) { return e && E(e)->isRunning(); }
double fls_engine_delta_time(FlsEngine e) { return e ? E(e)->deltaTime()   : 0.0; }
double fls_engine_elapsed_time(FlsEngine e) { return e ? E(e)->elapsedTime() : 0.0; }
float fls_engine_fps(FlsEngine e) { return e ? E(e)->fps() : 0.0f; }

// ---- Scene ----
FlsScene fls_scene_create(FlsEngine e, const char* /*name*/) {
    return e ? static_cast<FlsScene>(&E(e)->world()) : nullptr;
}
void fls_scene_destroy(FlsScene) {}  // world owned by Engine
FlsStatus fls_scene_load(FlsEngine e, const char* path) {
    if (!e || !path) return FLS_INVALID_ARG;
    return E(e)->world().loadFromFile(path) ? FLS_OK : FLS_ERR;
}
FlsStatus fls_scene_save(FlsScene s, const char* path) {
    if (!s || !path) return FLS_INVALID_ARG;
    return W(s)->saveToFile(path) ? FLS_OK : FLS_ERR;
}
FlsScene fls_scene_get_active(FlsEngine e) {
    return e ? static_cast<FlsScene>(&E(e)->world()) : nullptr;
}
const char* fls_scene_name(FlsScene) { return "Main"; }

// ---- Entity ----
FlsEntityID fls_entity_create(FlsScene s, const char* name) {
    return s ? W(s)->createEntity(name ? name : "Entity") : FLS_NULL_ENTITY;
}
void fls_entity_destroy(FlsScene s, FlsEntityID id) {
    if (s) W(s)->destroyEntity(id);
}
bool fls_entity_valid(FlsScene s, FlsEntityID id) {
    return s && W(s)->isValid(id);
}
const char* fls_entity_name(FlsScene s, FlsEntityID id) {
    if (!s) return "";
    static thread_local std::string n;
    n = W(s)->getName(id);
    return n.c_str();
}
void fls_entity_set_name(FlsScene s, FlsEntityID id, const char* name) {
    if (s && name) W(s)->setName(id, name);
}
void fls_entity_set_active(FlsScene s, FlsEntityID id, bool v) {
    if (s) W(s)->setActive(id, v);
}
bool fls_entity_is_active(FlsScene s, FlsEntityID id) {
    return s && W(s)->isActive(id);
}
FlsEntityID fls_entity_parent(FlsScene s, FlsEntityID id) {
    return s ? W(s)->getParent(id) : FLS_NULL_ENTITY;
}
void fls_entity_set_parent(FlsScene s, FlsEntityID child, FlsEntityID parent) {
    if (s) W(s)->setParent(child, parent);
}

// ---- Transform ----
FlsVec3 fls_transform_pos(FlsScene s, FlsEntityID id) {
    if (!s) return {};
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    return t ? FlsVec3{t->position.x,t->position.y,t->position.z} : FlsVec3{};
}
void fls_transform_set_pos(FlsScene s, FlsEntityID id, FlsVec3 p) {
    if (!s) return;
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    if (t) { t->position={p.x,p.y,p.z}; t->dirty=true; }
}
FlsQuat fls_transform_rot(FlsScene s, FlsEntityID id) {
    if (!s) return {0,0,0,1};
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    return t ? FlsQuat{t->rotation.x,t->rotation.y,t->rotation.z,t->rotation.w}
             : FlsQuat{0,0,0,1};
}
void fls_transform_set_rot(FlsScene s, FlsEntityID id, FlsQuat q) {
    if (!s) return;
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    if (t) { t->rotation={q.x,q.y,q.z,q.w}; t->dirty=true; }
}
FlsVec3 fls_transform_scale(FlsScene s, FlsEntityID id) {
    if (!s) return {1,1,1};
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    return t ? FlsVec3{t->scale.x,t->scale.y,t->scale.z} : FlsVec3{1,1,1};
}
void fls_transform_set_scale(FlsScene s, FlsEntityID id, FlsVec3 sc) {
    if (!s) return;
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    if (t) { t->scale={sc.x,sc.y,sc.z}; t->dirty=true; }
}
FlsMat4 fls_transform_world_matrix(FlsScene s, FlsEntityID id) {
    FlsMat4 out{}; // identity
    out.m[0]=1; out.m[5]=1; out.m[10]=1; out.m[15]=1;
    if (!s) return out;
    auto* t = W(s)->getComponent<Feliss::TransformComponent>(id);
    if (t) { for(int i=0;i<16;i++) out.m[i]=t->worldMatrix.m[i/4][i%4]; }
    return out;
}

// ---- Components ----
FlsComponent fls_component_add(FlsScene s, FlsEntityID id, FlsComponentType t) {
    if (!s) return nullptr;
    Feliss::World& w = *W(s);
    switch(t) {
        case FLS_COMP_MESH:      return &w.addComponent<Feliss::MeshRendererComponent>(id);
        case FLS_COMP_CAMERA:    return &w.addComponent<Feliss::CameraComponent>(id);
        case FLS_COMP_LIGHT:     return &w.addComponent<Feliss::LightComponent>(id);
        case FLS_COMP_RIGIDBODY: return &w.addComponent<Feliss::RigidBodyComponent>(id);
        case FLS_COMP_SCRIPT:    return &w.addComponent<Feliss::ScriptComponent>(id);
        case FLS_COMP_AUDIO:     return &w.addComponent<Feliss::AudioSourceComponent>(id);
        case FLS_COMP_SPRITE:    return &w.addComponent<Feliss::SpriteComponent>(id);
        default: return nullptr;
    }
}
FlsComponent fls_component_get(FlsScene s, FlsEntityID id, FlsComponentType t) {
    if (!s) return nullptr;
    Feliss::World& w = *W(s);
    switch(t) {
        case FLS_COMP_MESH:      return w.getComponent<Feliss::MeshRendererComponent>(id);
        case FLS_COMP_CAMERA:    return w.getComponent<Feliss::CameraComponent>(id);
        case FLS_COMP_LIGHT:     return w.getComponent<Feliss::LightComponent>(id);
        case FLS_COMP_RIGIDBODY: return w.getComponent<Feliss::RigidBodyComponent>(id);
        case FLS_COMP_SCRIPT:    return w.getComponent<Feliss::ScriptComponent>(id);
        case FLS_COMP_AUDIO:     return w.getComponent<Feliss::AudioSourceComponent>(id);
        case FLS_COMP_SPRITE:    return w.getComponent<Feliss::SpriteComponent>(id);
        default: return nullptr;
    }
}
bool fls_component_has(FlsScene s, FlsEntityID id, FlsComponentType t) {
    return fls_component_get(s, id, t) != nullptr;
}
void fls_component_remove(FlsScene s, FlsEntityID id, FlsComponentType t) {
    if (!s) return;
    Feliss::World& w = *W(s);
    switch(t) {
        case FLS_COMP_MESH:      w.removeComponent<Feliss::MeshRendererComponent>(id); break;
        case FLS_COMP_CAMERA:    w.removeComponent<Feliss::CameraComponent>(id);        break;
        case FLS_COMP_LIGHT:     w.removeComponent<Feliss::LightComponent>(id);         break;
        case FLS_COMP_RIGIDBODY: w.removeComponent<Feliss::RigidBodyComponent>(id);     break;
        case FLS_COMP_SCRIPT:    w.removeComponent<Feliss::ScriptComponent>(id);        break;
        case FLS_COMP_AUDIO:     w.removeComponent<Feliss::AudioSourceComponent>(id);   break;
        case FLS_COMP_SPRITE:    w.removeComponent<Feliss::SpriteComponent>(id);        break;
        default: break;
    }
}

// ---- Scripting ----
FlsStatus fls_lua_exec_file(FlsEngine e, const char* path) {
    if (!e || !path) return FLS_INVALID_ARG;
    return E(e)->scripts().execLua(path) ? FLS_OK : FLS_ERR;
}
FlsStatus fls_lua_exec_string(FlsEngine e, const char* code) {
    if (!e || !code) return FLS_INVALID_ARG;
    return E(e)->scripts().execLuaString(code) ? FLS_OK : FLS_ERR;
}
FlsStatus fls_csharp_load_assembly(FlsEngine e, const char* path) {
    if (!e || !path) return FLS_INVALID_ARG;
    return E(e)->scripts().loadCSharpAssembly(path) ? FLS_OK : FLS_ERR;
}

// ---- Input ----
bool fls_key_pressed(FlsEngine e, int key) {
    return e && E(e)->window().rawKey(static_cast<Feliss::KeyCode>(key));
}
bool fls_key_held(FlsEngine e, int key) { return fls_key_pressed(e, key); }
bool fls_key_released(FlsEngine, int)   { return false; } // needs InputSystem state machine
bool fls_mouse_button(FlsEngine e, int btn) {
    return e && E(e)->window().rawMouseButton(static_cast<Feliss::MouseButton>(btn));
}
FlsVec2 fls_mouse_pos(FlsEngine e) {
    if (!e) return {};
    auto p = E(e)->window().rawMousePos();
    return {p.x, p.y};
}
FlsVec2 fls_mouse_delta(FlsEngine, int) { return {}; }
FlsVec2 fls_mouse_delta(FlsEngine e) {
    (void)e; return {};
}
float fls_mouse_scroll(FlsEngine e) {
    if (!e) return 0.f;
    return E(e)->window().rawMouseScroll().y;
}

// ---- Assets ----
FlsAssetID fls_asset_load(FlsEngine, const char*) { return FLS_NULL_ASSET; }
void       fls_asset_unload(FlsEngine, FlsAssetID) {}
bool       fls_asset_loaded(FlsEngine, FlsAssetID) { return false; }
const char* fls_asset_path(FlsEngine, FlsAssetID) { return ""; }

// ---- Extensions ----
FlsStatus fls_ext_load(FlsEngine e, const char* path) {
    if (!e || !path) return FLS_INVALID_ARG;
    return E(e)->extensions().load(path) ? FLS_OK : FLS_ERR;
}
FlsStatus fls_ext_unload(FlsEngine e, const char* id) {
    if (!e || !id) return FLS_INVALID_ARG;
    return E(e)->extensions().unload(id) ? FLS_OK : FLS_NOT_FOUND;
}
void fls_ext_enable(FlsEngine e, const char* id)  { if (e && id) E(e)->extensions().setEnabled(id,true);  }
void fls_ext_disable(FlsEngine e, const char* id) { if (e && id) E(e)->extensions().setEnabled(id,false); }
bool fls_ext_is_enabled(FlsEngine e, const char* id) {
    return e && id && E(e)->extensions().isEnabled(id);
}
int  fls_ext_count(FlsEngine e) { return e ? E(e)->extensions().count() : 0; }
const char* fls_ext_id_at(FlsEngine e, int idx) {
    if (!e || idx < 0 || idx >= E(e)->extensions().count()) return "";
    return E(e)->extensions().all()[idx].info.id.c_str();
}

// ---- Log ----
void fls_log(FlsLogLevel lvl, const char* tag, const char* msg) {
    Feliss::Logger::get().log(static_cast<Feliss::LogLevel>(lvl),
        tag ? tag : "C", msg ? msg : "");
}
void fls_log_set_callback(FlsLogCb cb, void* ud) {
    Feliss::Logger::get().addCallback([cb, ud](const Feliss::LogEntry& e){
        if (cb) cb(static_cast<FlsLogLevel>(e.level), e.tag.c_str(), e.message.c_str(), ud);
    });
}
void fls_log_set_level(FlsLogLevel min) {
    Feliss::Logger::get().setMinLevel(static_cast<Feliss::LogLevel>(min));
}

} // extern "C"
