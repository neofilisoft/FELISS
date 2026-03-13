#pragma once
#include "feliss/Types.h"
#include <string>
#include <vector>
#include <functional>
#include <variant>
#include <memory>

namespace Feliss {

class Engine;

// =====================================================================
// ScriptValue — universal value that crosses the script boundary
// =====================================================================
using ScriptValue = std::variant<std::monostate, bool, i64, f64, std::string>;

// =====================================================================
// IScriptBackend — interface for C# and Lua backends
// =====================================================================
class IScriptBackend {
public:
    virtual ~IScriptBackend() = default;

    virtual const char* name()   const = 0;
    virtual bool        init()         = 0;
    virtual void        shutdown()     = 0;
    virtual void        update(f32 dt) {}

    virtual bool loadFile(const std::string& path) = 0;
    virtual bool loadString(const std::string& code,
                            const std::string& chunkName = "<string>") = 0;

    // Instance lifecycle (for script components)
    virtual void* createInstance(const std::string& className)  = 0;
    virtual void  destroyInstance(void* handle)                 = 0;

    // Call a method on an instance or a global function
    virtual bool callMethod(void* handle, const std::string& method,
                            const std::vector<ScriptValue>& args = {},
                            ScriptValue* out = nullptr)            = 0;
    virtual bool callGlobal(const std::string& fn,
                            const std::vector<ScriptValue>& args = {},
                            ScriptValue* out = nullptr)            = 0;

    // Property access on an instance
    virtual bool        setProp(void* handle, const std::string& n, const ScriptValue& v) = 0;
    virtual ScriptValue getProp(void* handle, const std::string& n) = 0;

    // Bind engine API so scripts can call engine functions
    virtual void bindAPI(Engine& engine) = 0;

    // Hot-reload: re-load all previously loaded files
    virtual void reload() {}

    std::string lastError() const { return m_lastError; }

protected:
    std::string m_lastError;
};

// =====================================================================
// ScriptEngineConfig
// =====================================================================
struct ScriptEngineConfig {
    bool        enableLua       = true;
    bool        enableCSharp    = false;
    std::string luaPath         = "./assets/scripts/?.lua;./assets/scripts/?/init.lua";
    std::string csharpAssembly  = "";   // path to compiled .dll
};

// =====================================================================
// ScriptEngine — owns and routes to C# / Lua backends
// =====================================================================
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();
    ScriptEngine(const ScriptEngine&)            = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;

    bool init(const ScriptEngineConfig& cfg);
    void shutdown();
    void update(f32 dt);

    // Quick exec
    bool execLua(const std::string& path);
    bool execLuaString(const std::string& code);
    bool loadCSharpAssembly(const std::string& path);

    // Script component hooks
    void onStart  (EntityID id, void* handle, bool isLua);
    void onUpdate (EntityID id, void* handle, bool isLua, f32 dt);
    void onDestroy(EntityID id, void* handle, bool isLua);

    // Reload all scripts (hot-reload)
    void reloadAll();

    // Register a native C++ function callable from scripts
    using NativeFn = std::function<ScriptValue(std::vector<ScriptValue>)>;
    void registerNative(const std::string& ns, const std::string& n, NativeFn fn);

    IScriptBackend* lua()    const { return m_lua.get();    }
    IScriptBackend* csharp() const { return m_csharp.get(); }

    bool isLuaEnabled()    const { return m_luaEnabled; }
    bool isCSharpEnabled() const { return m_csEnabled;  }

private:
    std::unique_ptr<IScriptBackend> m_lua;
    std::unique_ptr<IScriptBackend> m_csharp;
    bool m_luaEnabled = false;
    bool m_csEnabled  = false;
};

} // namespace Feliss
