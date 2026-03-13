#include "scripting/LuaBridge.h"
#include "core/Engine.h"
#include "core/Logger.h"
#include "ecs/World.h"
#include "ecs/Component.h"

#ifdef FELISS_LUA_ENABLED
extern "C" {
#  include <lua.h>
#  include <lualib.h>
#  include <lauxlib.h>
}
#endif

namespace Feliss {

#ifdef FELISS_LUA_ENABLED

// ---- Registry key for engine pointer ----
static const char* LUA_ENGINE_KEY = "__fls_engine";

static Engine& getEng(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_ENGINE_KEY);
    Engine* e = static_cast<Engine*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return *e;
}

// ============================================================
// Log API
// ============================================================
static int lua_log_info (lua_State* L) { FLS_INFO ("Lua", luaL_checkstring(L,1)); return 0; }
static int lua_log_warn (lua_State* L) { FLS_WARN ("Lua", luaL_checkstring(L,1)); return 0; }
static int lua_log_error(lua_State* L) { FLS_ERROR("Lua", luaL_checkstring(L,1)); return 0; }
static int lua_log_debug(lua_State* L) { FLS_DEBUG("Lua", luaL_checkstring(L,1)); return 0; }

// ============================================================
// Entity API
// ============================================================
static int lua_entity_create(lua_State* L) {
    const char* n = luaL_optstring(L, 1, "Entity");
    EntityID id = getEng(L).world().createEntity(n);
    lua_pushinteger(L, static_cast<lua_Integer>(id));
    return 1;
}
static int lua_entity_destroy(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    getEng(L).world().destroyEntity(id);
    return 0;
}
static int lua_entity_find(lua_State* L) {
    const char* n = luaL_checkstring(L, 1);
    EntityID id = getEng(L).world().findByName(n);
    if (id == NULL_ENTITY) { lua_pushnil(L); }
    else { lua_pushinteger(L, static_cast<lua_Integer>(id)); }
    return 1;
}
static int lua_entity_set_active(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    bool active = lua_toboolean(L, 2) != 0;
    getEng(L).world().setActive(id, active);
    return 0;
}
static int lua_entity_is_active(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, getEng(L).world().isActive(id) ? 1 : 0);
    return 1;
}
static int lua_entity_get_name(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    std::string n = getEng(L).world().getName(id);
    lua_pushstring(L, n.c_str());
    return 1;
}

// ============================================================
// Transform API
// ============================================================
static int lua_transform_set_pos(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    f32 x = static_cast<f32>(luaL_checknumber(L, 2));
    f32 y = static_cast<f32>(luaL_checknumber(L, 3));
    f32 z = static_cast<f32>(luaL_optnumber(L, 4, 0.0));
    auto* t = getEng(L).world().getComponent<TransformComponent>(id);
    if (t) { t->position = {x,y,z}; t->dirty = true; }
    return 0;
}
static int lua_transform_get_pos(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    auto* t = getEng(L).world().getComponent<TransformComponent>(id);
    if (!t) { lua_pushnil(L); return 1; }
    lua_newtable(L);
    lua_pushnumber(L, t->position.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, t->position.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, t->position.z); lua_setfield(L, -2, "z");
    return 1;
}
static int lua_transform_set_scale(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    f32 x = static_cast<f32>(luaL_checknumber(L, 2));
    f32 y = static_cast<f32>(luaL_checknumber(L, 3));
    f32 z = static_cast<f32>(luaL_optnumber(L, 4, 1.0));
    auto* t = getEng(L).world().getComponent<TransformComponent>(id);
    if (t) { t->scale = {x,y,z}; t->dirty = true; }
    return 0;
}
static int lua_transform_translate(lua_State* L) {
    EntityID id = static_cast<EntityID>(luaL_checkinteger(L, 1));
    f32 x = static_cast<f32>(luaL_checknumber(L, 2));
    f32 y = static_cast<f32>(luaL_checknumber(L, 3));
    f32 z = static_cast<f32>(luaL_optnumber(L, 4, 0.0));
    auto* t = getEng(L).world().getComponent<TransformComponent>(id);
    if (t) { t->position.x += x; t->position.y += y; t->position.z += z; t->dirty = true; }
    return 0;
}

// ============================================================
// Input API
// ============================================================
static int lua_input_key_down(lua_State* L) {
    int key = static_cast<int>(luaL_checkinteger(L, 1));
    bool down = getEng(L).window().rawKey(static_cast<KeyCode>(key));
    lua_pushboolean(L, down ? 1 : 0);
    return 1;
}
static int lua_input_mouse_pos(lua_State* L) {
    Vec2 pos = getEng(L).window().rawMousePos();
    lua_newtable(L);
    lua_pushnumber(L, pos.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, pos.y); lua_setfield(L, -2, "y");
    return 1;
}

// ============================================================
// Time API
// ============================================================
static int lua_time_delta(lua_State* L) {
    lua_pushnumber(L, static_cast<lua_Number>(getEng(L).deltaTime()));
    return 1;
}
static int lua_time_elapsed(lua_State* L) {
    lua_pushnumber(L, static_cast<lua_Number>(getEng(L).elapsedTime()));
    return 1;
}
static int lua_time_fps(lua_State* L) {
    lua_pushnumber(L, static_cast<lua_Number>(getEng(L).fps()));
    return 1;
}

#endif // FELISS_LUA_ENABLED

// ============================================================
// LuaBridge implementation
// ============================================================

LuaBridge::LuaBridge()  = default;
LuaBridge::~LuaBridge() { shutdown(); }

bool LuaBridge::init() {
#ifdef FELISS_LUA_ENABLED
    m_L = luaL_newstate();
    if (!m_L) { m_lastError = "luaL_newstate failed"; return false; }
    luaL_openlibs(m_L);
    FLS_INFOF("Lua", "Lua " LUA_RELEASE " initialized");
    return true;
#else
    m_lastError = "Lua not compiled in (missing FELISS_LUA_ENABLED)";
    FLS_WARN("Lua", m_lastError);
    return false;
#endif
}

void LuaBridge::shutdown() {
#ifdef FELISS_LUA_ENABLED
    if (m_L) { lua_close(m_L); m_L = nullptr; FLS_INFO("Lua", "Lua closed"); }
#endif
}

void LuaBridge::update(f32) {
#ifdef FELISS_LUA_ENABLED
    if (m_L) lua_gc(m_L, LUA_GCSTEP, 10);
#endif
}

bool LuaBridge::loadFile(const std::string& path) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L) return false;
    int rc = luaL_loadfile(m_L, path.c_str());
    if (rc == LUA_OK) rc = lua_pcall(m_L, 0, LUA_MULTRET, 0);
    if (handleResult(rc)) {
        m_loadedFiles.push_back(path);
        return true;
    }
    return false;
#else
    return false;
#endif
}

bool LuaBridge::loadString(const std::string& code, const std::string& chunkName) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L) return false;
    int rc = luaL_loadbuffer(m_L, code.c_str(), code.size(), chunkName.c_str());
    if (rc == LUA_OK) rc = lua_pcall(m_L, 0, LUA_MULTRET, 0);
    return handleResult(rc);
#else
    return false;
#endif
}

void* LuaBridge::createInstance(const std::string& className) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L) return nullptr;
    lua_getglobal(m_L, className.c_str());
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        m_lastError = "Class not found: " + className;
        return nullptr;
    }
    // Create instance table with __index pointing to class
    lua_newtable(m_L);               // instance table
    lua_pushvalue(m_L, -2);          // class table
    lua_setfield(m_L, -2, "__index");
    int ref = luaL_ref(m_L, LUA_REGISTRYINDEX);
    lua_pop(m_L, 1); // pop class
    return reinterpret_cast<void*>(static_cast<intptr_t>(ref));
#else
    return nullptr;
#endif
}

void LuaBridge::destroyInstance(void* handle) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L || !handle) return;
    int ref = static_cast<int>(reinterpret_cast<intptr_t>(handle));
    luaL_unref(m_L, LUA_REGISTRYINDEX, ref);
#endif
}

bool LuaBridge::callMethod(void* handle, const std::string& method,
                            const std::vector<ScriptValue>& args, ScriptValue* out) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L || !handle) return false;
    int ref = static_cast<int>(reinterpret_cast<intptr_t>(handle));
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
    lua_getfield(m_L, -1, method.c_str());
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 2);
        return false;
    }
    lua_pushvalue(m_L, -2); // self
    for (auto& a : args) pushValue(a);
    int rc = lua_pcall(m_L, static_cast<int>(args.size()) + 1, out ? 1 : 0, 0);
    if (!handleResult(rc)) { lua_pop(m_L, 1); return false; }
    if (out) *out = popValue();
    lua_pop(m_L, 1); // instance table
    return true;
#else
    return false;
#endif
}

bool LuaBridge::callGlobal(const std::string& fn,
                            const std::vector<ScriptValue>& args, ScriptValue* out) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L) return false;
    lua_getglobal(m_L, fn.c_str());
    if (!lua_isfunction(m_L, -1)) { lua_pop(m_L, 1); return false; }
    for (auto& a : args) pushValue(a);
    int rc = lua_pcall(m_L, static_cast<int>(args.size()), out ? 1 : 0, 0);
    if (!handleResult(rc)) return false;
    if (out) *out = popValue();
    return true;
#else
    return false;
#endif
}

bool LuaBridge::setProp(void* handle, const std::string& n, const ScriptValue& v) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L || !handle) return false;
    int ref = static_cast<int>(reinterpret_cast<intptr_t>(handle));
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
    pushValue(v);
    lua_setfield(m_L, -2, n.c_str());
    lua_pop(m_L, 1);
    return true;
#else
    return false;
#endif
}

ScriptValue LuaBridge::getProp(void* handle, const std::string& n) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L || !handle) return std::monostate{};
    int ref = static_cast<int>(reinterpret_cast<intptr_t>(handle));
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
    lua_getfield(m_L, -1, n.c_str());
    auto v = popValue();
    lua_pop(m_L, 1);
    return v;
#else
    return std::monostate{};
#endif
}

void LuaBridge::bindAPI(Engine& engine) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L) return;
    m_engine = &engine;
    // Store engine pointer in Lua registry
    lua_pushlightuserdata(m_L, &engine);
    lua_setfield(m_L, LUA_REGISTRYINDEX, LUA_ENGINE_KEY);

    bindLogAPI();
    bindEntityAPI();
    bindTransformAPI();
    bindInputAPI();
    bindTimeAPI();
    bindMathAPI();

    // Override print → Log.info
    lua_pushcfunction(m_L, lua_log_info);
    lua_setglobal(m_L, "print");

    FLS_INFO("Lua", "Engine API bound to Lua");
#endif
}

void LuaBridge::reload() {
#ifdef FELISS_LUA_ENABLED
    auto files = m_loadedFiles;
    for (auto& f : files) loadFile(f);
    FLS_INFOF("Lua", "Reloaded " << files.size() << " scripts");
#endif
}

void LuaBridge::registerFn(const std::string& table, const std::string& fn, LuaCFn f) {
#ifdef FELISS_LUA_ENABLED
    if (!m_L) return;
    lua_getglobal(m_L, table.c_str());
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        lua_newtable(m_L);
    }
    lua_pushcfunction(m_L, f);
    lua_setfield(m_L, -2, fn.c_str());
    lua_setglobal(m_L, table.c_str());
#endif
}

// ---- Private helpers ----
void LuaBridge::bindLogAPI() {
#ifdef FELISS_LUA_ENABLED
    lua_newtable(m_L);
    lua_pushcfunction(m_L, lua_log_info);  lua_setfield(m_L, -2, "info");
    lua_pushcfunction(m_L, lua_log_warn);  lua_setfield(m_L, -2, "warn");
    lua_pushcfunction(m_L, lua_log_error); lua_setfield(m_L, -2, "error");
    lua_pushcfunction(m_L, lua_log_debug); lua_setfield(m_L, -2, "debug");
    lua_setglobal(m_L, "Log");
#endif
}
void LuaBridge::bindEntityAPI() {
#ifdef FELISS_LUA_ENABLED
    lua_newtable(m_L);
    lua_pushcfunction(m_L, lua_entity_create);     lua_setfield(m_L, -2, "create");
    lua_pushcfunction(m_L, lua_entity_destroy);    lua_setfield(m_L, -2, "destroy");
    lua_pushcfunction(m_L, lua_entity_find);       lua_setfield(m_L, -2, "find");
    lua_pushcfunction(m_L, lua_entity_set_active); lua_setfield(m_L, -2, "setActive");
    lua_pushcfunction(m_L, lua_entity_is_active);  lua_setfield(m_L, -2, "isActive");
    lua_pushcfunction(m_L, lua_entity_get_name);   lua_setfield(m_L, -2, "getName");
    lua_setglobal(m_L, "Entity");
#endif
}
void LuaBridge::bindTransformAPI() {
#ifdef FELISS_LUA_ENABLED
    lua_newtable(m_L);
    lua_pushcfunction(m_L, lua_transform_set_pos);  lua_setfield(m_L, -2, "setPosition");
    lua_pushcfunction(m_L, lua_transform_get_pos);  lua_setfield(m_L, -2, "getPosition");
    lua_pushcfunction(m_L, lua_transform_set_scale);lua_setfield(m_L, -2, "setScale");
    lua_pushcfunction(m_L, lua_transform_translate);lua_setfield(m_L, -2, "translate");
    lua_setglobal(m_L, "Transform");
#endif
}
void LuaBridge::bindInputAPI() {
#ifdef FELISS_LUA_ENABLED
    lua_newtable(m_L);
    lua_pushcfunction(m_L, lua_input_key_down); lua_setfield(m_L, -2, "isKeyDown");
    lua_pushcfunction(m_L, lua_input_mouse_pos);lua_setfield(m_L, -2, "mousePos");
    lua_setglobal(m_L, "Input");

    // Key code constants
    lua_newtable(m_L);
#define LUA_KEY(name, val) lua_pushinteger(m_L,val); lua_setfield(m_L,-2, #name);
    LUA_KEY(Space,   32)  LUA_KEY(A,65) LUA_KEY(B,66) LUA_KEY(C,67)
    LUA_KEY(D,68)  LUA_KEY(E,69) LUA_KEY(F,70) LUA_KEY(G,71)
    LUA_KEY(H,72)  LUA_KEY(I,73) LUA_KEY(J,74) LUA_KEY(K,75)
    LUA_KEY(L,76)  LUA_KEY(M,77) LUA_KEY(N,78) LUA_KEY(O,79)
    LUA_KEY(P,80)  LUA_KEY(Q,81) LUA_KEY(R,82) LUA_KEY(S,83)
    LUA_KEY(T,84)  LUA_KEY(U,85) LUA_KEY(V,86) LUA_KEY(W,87)
    LUA_KEY(X,88)  LUA_KEY(Y,89) LUA_KEY(Z,90)
    LUA_KEY(Up,265) LUA_KEY(Down,264) LUA_KEY(Left,263) LUA_KEY(Right,262)
    LUA_KEY(Enter,257) LUA_KEY(Escape,256) LUA_KEY(Tab,258)
    LUA_KEY(LeftShift,340) LUA_KEY(LeftCtrl,341) LUA_KEY(LeftAlt,342)
#undef LUA_KEY
    lua_setglobal(m_L, "Key");
#endif
}
void LuaBridge::bindTimeAPI() {
#ifdef FELISS_LUA_ENABLED
    lua_newtable(m_L);
    lua_pushcfunction(m_L, lua_time_delta);   lua_setfield(m_L, -2, "delta");
    lua_pushcfunction(m_L, lua_time_elapsed); lua_setfield(m_L, -2, "elapsed");
    lua_pushcfunction(m_L, lua_time_fps);     lua_setfield(m_L, -2, "fps");
    lua_setglobal(m_L, "Time");
#endif
}
void LuaBridge::bindMathAPI() {
    // Lua already has a math library; we just extend it
}

void LuaBridge::pushValue(const ScriptValue& v) {
#ifdef FELISS_LUA_ENABLED
    std::visit([&](auto&& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::monostate>)
            lua_pushnil(m_L);
        else if constexpr (std::is_same_v<T, bool>)
            lua_pushboolean(m_L, val ? 1 : 0);
        else if constexpr (std::is_same_v<T, i64>)
            lua_pushinteger(m_L, static_cast<lua_Integer>(val));
        else if constexpr (std::is_same_v<T, f64>)
            lua_pushnumber(m_L, static_cast<lua_Number>(val));
        else if constexpr (std::is_same_v<T, std::string>)
            lua_pushstring(m_L, val.c_str());
    }, v);
#endif
}

ScriptValue LuaBridge::popValue() {
#ifdef FELISS_LUA_ENABLED
    if (!m_L || lua_gettop(m_L) == 0) return std::monostate{};
    ScriptValue v;
    switch (lua_type(m_L, -1)) {
        case LUA_TBOOLEAN: v = (lua_toboolean(m_L, -1) != 0);         break;
        case LUA_TNUMBER:
            if (lua_isinteger(m_L, -1))
                 v = static_cast<i64>(lua_tointeger(m_L, -1));
            else v = static_cast<f64>(lua_tonumber(m_L, -1));
            break;
        case LUA_TSTRING: v = std::string(lua_tostring(m_L, -1));     break;
        default:          v = std::monostate{};                        break;
    }
    lua_pop(m_L, 1);
    return v;
#else
    return std::monostate{};
#endif
}

bool LuaBridge::handleResult(int rc) {
#ifdef FELISS_LUA_ENABLED
    if (rc == LUA_OK) return true;
    if (m_L && !lua_isnil(m_L, -1)) {
        const char* err = lua_tostring(m_L, -1);
        m_lastError = err ? err : "unknown Lua error";
        lua_pop(m_L, 1);
        FLS_ERROR("Lua", m_lastError);
    }
    return false;
#else
    return false;
#endif
}

} // namespace Feliss
