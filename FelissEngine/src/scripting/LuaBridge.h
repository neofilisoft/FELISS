#pragma once
#include "scripting/ScriptEngine.h"

struct lua_State;

namespace Feliss {

// =====================================================================
// LuaBridge — Lua 5.4 scripting backend
//
// Script API exposed to Lua:
//   Entity.create(name)  → id
//   Entity.destroy(id)
//   Entity.find(name)    → id or nil
//   Transform.setPosition(id, x, y, z)
//   Transform.getPosition(id) → {x,y,z}
//   Transform.setScale(id, x, y, z)
//   Input.isKeyDown(keyCode)  → bool
//   Time.delta()  → float
//   Time.elapsed()→ float
//   Log.info(msg), Log.warn(msg), Log.error(msg)
// =====================================================================
class LuaBridge : public IScriptBackend {
public:
    LuaBridge();
    ~LuaBridge() override;

    const char* name()       const override { return "Lua 5.4"; }
    bool        init()             override;
    void        shutdown()         override;
    void        update(f32 dt)     override;

    bool loadFile(const std::string& path) override;
    bool loadString(const std::string& code,
                    const std::string& chunkName) override;

    void* createInstance(const std::string& className) override;
    void  destroyInstance(void* handle)                override;

    bool callMethod(void* handle, const std::string& method,
                    const std::vector<ScriptValue>& args,
                    ScriptValue* out) override;
    bool callGlobal(const std::string& fn,
                    const std::vector<ScriptValue>& args,
                    ScriptValue* out) override;

    bool        setProp(void* handle, const std::string& n, const ScriptValue& v) override;
    ScriptValue getProp(void* handle, const std::string& n) override;

    void bindAPI(Engine& engine) override;
    void reload() override;

    lua_State* state() const { return m_L; }

    // Register a C function into a Lua table: tableName.funcName = fn
    using LuaCFn = int(*)(lua_State*);
    void registerFn(const std::string& table, const std::string& fn, LuaCFn f);

private:
    void pushValue(const ScriptValue& v);
    ScriptValue popValue();
    bool handleResult(int rc);

    void bindLogAPI();
    void bindEntityAPI();
    void bindTransformAPI();
    void bindInputAPI();
    void bindTimeAPI();
    void bindMathAPI();

    lua_State* m_L      = nullptr;
    Engine*    m_engine = nullptr;
    std::vector<std::string> m_loadedFiles;
};

} // namespace Feliss
