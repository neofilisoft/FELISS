#include "scripting/ScriptEngine.h"
#include "scripting/LuaBridge.h"
#include "scripting/CSharpBridge.h"
#include "core/Logger.h"

namespace Feliss {

ScriptEngine::ScriptEngine()  = default;
ScriptEngine::~ScriptEngine() { shutdown(); }

bool ScriptEngine::init(const ScriptEngineConfig& cfg) {
    if (cfg.enableLua) {
        m_lua = std::make_unique<LuaBridge>();
        if (m_lua->init()) {
            m_luaEnabled = true;
            FLS_INFO("ScriptEngine", "Lua backend ready");
        } else {
            FLS_WARN("ScriptEngine", "Lua init failed: " + m_lua->lastError());
            m_lua.reset();
        }
    }
    if (cfg.enableCSharp) {
        m_csharp = std::make_unique<CSharpBridge>();
        if (m_csharp->init()) {
            m_csEnabled = true;
            if (!cfg.csharpAssembly.empty())
                static_cast<CSharpBridge*>(m_csharp.get())->loadAssembly(cfg.csharpAssembly);
            FLS_INFO("ScriptEngine", "C# backend ready");
        } else {
            FLS_WARN("ScriptEngine", "C# init failed: " + m_csharp->lastError());
            m_csharp.reset();
        }
    }
    return m_luaEnabled || m_csEnabled || (!cfg.enableLua && !cfg.enableCSharp);
}

void ScriptEngine::shutdown() {
    if (m_csharp) { m_csharp->shutdown(); m_csharp.reset(); m_csEnabled = false; }
    if (m_lua)    { m_lua->shutdown();    m_lua.reset();    m_luaEnabled = false; }
}

void ScriptEngine::update(f32 dt) {
    if (m_lua)    m_lua->update(dt);
    if (m_csharp) m_csharp->update(dt);
}

bool ScriptEngine::execLua(const std::string& path) {
    if (!m_lua) { FLS_WARN("ScriptEngine", "Lua not enabled"); return false; }
    return m_lua->loadFile(path);
}
bool ScriptEngine::execLuaString(const std::string& code) {
    if (!m_lua) return false;
    return m_lua->loadString(code, "<runtime>");
}
bool ScriptEngine::loadCSharpAssembly(const std::string& path) {
    if (!m_csharp) { FLS_WARN("ScriptEngine", "C# not enabled"); return false; }
    return static_cast<CSharpBridge*>(m_csharp.get())->loadAssembly(path);
}

void ScriptEngine::onStart(EntityID id, void* handle, bool isLua) {
    auto* backend = isLua ? m_lua.get() : m_csharp.get();
    if (!backend || !handle) return;
    ScriptValue idVal = static_cast<i64>(id);
    backend->callMethod(handle, "onStart", {idVal});
}
void ScriptEngine::onUpdate(EntityID id, void* handle, bool isLua, f32 dt) {
    auto* backend = isLua ? m_lua.get() : m_csharp.get();
    if (!backend || !handle) return;
    backend->callMethod(handle, "onUpdate", {static_cast<i64>(id), static_cast<f64>(dt)});
}
void ScriptEngine::onDestroy(EntityID id, void* handle, bool isLua) {
    auto* backend = isLua ? m_lua.get() : m_csharp.get();
    if (!backend || !handle) return;
    backend->callMethod(handle, "onDestroy", {static_cast<i64>(id)});
}
void ScriptEngine::reloadAll() {
    if (m_lua)    m_lua->reload();
    if (m_csharp) m_csharp->reload();
    FLS_INFO("ScriptEngine", "All scripts reloaded");
}
void ScriptEngine::registerNative(const std::string&, const std::string&, NativeFn) {
    // TODO: expose through Lua C API or Mono internal calls
}

} // namespace Feliss
