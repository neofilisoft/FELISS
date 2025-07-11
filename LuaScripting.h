#pragma once

#include <string>
#include <memory>
#include <lua.hpp>  

namespace FelissScript {

class LuaScriptingEngine {
public:
    LuaScriptingEngine();
    ~LuaScriptingEngine();

    bool LoadScript(const std::string& path);

    void CallFunction(const std::string& funcName);

    void SetGlobal(const std::string& name, int value);
    void SetGlobal(const std::string& name, float value);
    void SetGlobal(const std::string& name, const std::string& value);

private:
    lua_State* L;  // ตัว Lua context

    void ReportErrors(int status);
};

} 
