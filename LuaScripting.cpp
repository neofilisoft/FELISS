#include "FelissScript/LuaScripting.h"
#include <iostream>
#include <fstream>

namespace FelissScript {

LuaScriptingEngine::LuaScriptingEngine() {
    L = luaL_newstate();
    luaL_openlibs(L); 
}

LuaScriptingEngine::~LuaScriptingEngine() {
    if (L) lua_close(L);
}

bool LuaScriptingEngine::LoadScript(const std::string& path) {
    int status = luaL_dofile(L, path.c_str());
    if (status != LUA_OK) {
        ReportErrors(status);
        return false;
    }
    return true;
}

void LuaScriptingEngine::CallFunction(const std::string& funcName) {
    lua_getglobal(L, funcName.c_str());
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            ReportErrors(lua_status(L));
    } else {
        lua_pop(L, 1); // remove non-function
    }
}

void LuaScriptingEngine::SetGlobal(const std::string& name, int value) {
    lua_pushinteger(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaScriptingEngine::SetGlobal(const std::string& name, float value) {
    lua_pushnumber(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaScriptingEngine::SetGlobal(const std::string& name, const std::string& value) {
    lua_pushstring(L, value.c_str());
    lua_setglobal(L, name.c_str());
}

void LuaScriptingEngine::ReportErrors(int status) {
    const char* msg = lua_tostring(L, -1);
    if (msg) {
        std::cerr << "[Lua Error] " << msg << std::endl;
        lua_pop(L, 1);
    }
}


ScriptComponent::ScriptComponent(const std::string& path)
    : scriptPath(path), engine(std::make_shared<LuaScriptingEngine>()) {}

void ScriptComponent::OnCreate() {
    engine->LoadScript(scriptPath);
    engine->CallFunction("OnCreate");
}

void ScriptComponent::OnUpdate(float dt) {
    engine->SetGlobal("deltaTime", dt);
    engine->CallFunction("OnUpdate");
}

} 
