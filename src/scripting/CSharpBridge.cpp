#include "scripting/CSharpBridge.h"
#include "core/Engine.h"
#include "core/Logger.h"

#ifdef FELISS_CSHARP_ENABLED
#  include <mono/jit/jit.h>
#  include <mono/metadata/assembly.h>
#  include <mono/metadata/mono-config.h>
#  include <mono/metadata/object.h>
#  include <mono/metadata/threads.h>
#endif

namespace Feliss {

CSharpBridge::CSharpBridge()  = default;
CSharpBridge::~CSharpBridge() { shutdown(); }

bool CSharpBridge::init() {
#ifdef FELISS_CSHARP_ENABLED
    mono_config_parse(nullptr);
    m_domain = mono_jit_init("FelissScripting");
    if (!m_domain) {
        m_lastError = "mono_jit_init failed";
        FLS_ERROR("C#", m_lastError);
        return false;
    }
    FLS_INFO("C#", "Mono JIT initialized");
    return true;
#else
    m_lastError = "C# not compiled in — enable FELISS_CSHARP_ENABLED and link Mono";
    FLS_WARN("C#", m_lastError);
    return false;
#endif
}

void CSharpBridge::shutdown() {
#ifdef FELISS_CSHARP_ENABLED
    if (m_domain) {
        mono_jit_cleanup(static_cast<MonoDomain*>(m_domain));
        m_domain = nullptr;
        FLS_INFO("C#", "Mono shutdown");
    }
#endif
}

void CSharpBridge::update(f32) {}

bool CSharpBridge::loadFile(const std::string& path) {
    return loadAssembly(path);
}

bool CSharpBridge::loadString(const std::string&, const std::string&) {
    FLS_WARN("C#", "Runtime string compilation not supported — compile to .dll first");
    return false;
}

bool CSharpBridge::loadAssembly(const std::string& dllPath) {
#ifdef FELISS_CSHARP_ENABLED
    if (!m_domain) return false;
    MonoAssembly* asm_ = mono_domain_assembly_open(
        static_cast<MonoDomain*>(m_domain), dllPath.c_str());
    if (!asm_) {
        m_lastError = "Failed to open assembly: " + dllPath;
        FLS_ERROR("C#", m_lastError);
        return false;
    }
    m_assembly = asm_;
    m_image    = mono_assembly_get_image(asm_);
    m_loadedAssemblies.push_back(dllPath);
    FLS_INFOF("C#", "Loaded assembly: " << dllPath);
    return true;
#else
    FLS_WARN("C#", "loadAssembly: Mono not enabled");
    return false;
#endif
}

void* CSharpBridge::createInstance(const std::string& className) {
#ifdef FELISS_CSHARP_ENABLED
    if (!m_image) return nullptr;
    std::string ns, cls;
    auto dot = className.rfind('.');
    if (dot != std::string::npos) { ns = className.substr(0, dot); cls = className.substr(dot+1); }
    else { cls = className; }
    MonoClass* klass = mono_class_from_name(static_cast<MonoImage*>(m_image), ns.c_str(), cls.c_str());
    if (!klass) { m_lastError = "Class not found: " + className; return nullptr; }
    MonoObject* obj = mono_object_new(static_cast<MonoDomain*>(m_domain), klass);
    if (!obj) return nullptr;
    mono_runtime_object_init(obj);
    uint32_t h = mono_gchandle_new(obj, false);
    return reinterpret_cast<void*>(static_cast<uintptr_t>(h));
#else
    return nullptr;
#endif
}

void CSharpBridge::destroyInstance(void* handle) {
#ifdef FELISS_CSHARP_ENABLED
    if (!handle) return;
    mono_gchandle_free(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
#endif
}

bool CSharpBridge::callMethod(void* handle, const std::string& method,
                               const std::vector<ScriptValue>& args, ScriptValue* /*out*/) {
#ifdef FELISS_CSHARP_ENABLED
    if (!handle || !m_image) return false;
    MonoObject* obj = mono_gchandle_get_target(
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
    if (!obj) return false;
    MonoClass*  klass  = mono_object_get_class(obj);
    MonoMethod* meth   = mono_class_get_method_from_name(klass, method.c_str(),
                                                          (int)args.size());
    if (!meth) return false;
    MonoObject* exc = nullptr;
    mono_runtime_invoke(meth, obj, nullptr, &exc);
    if (exc) { FLS_ERROR("C#", "Exception in " + method); return false; }
    return true;
#else
    return false;
#endif
}

bool CSharpBridge::callGlobal(const std::string& fn,
                               const std::vector<ScriptValue>&, ScriptValue*) {
    FLS_WARNF("C#", "callGlobal not supported: " << fn);
    return false;
}

bool CSharpBridge::setProp(void*, const std::string&, const ScriptValue&) { return false; }
ScriptValue CSharpBridge::getProp(void*, const std::string&) { return std::monostate{}; }

void CSharpBridge::bindAPI(Engine& engine) {
    m_engine = &engine;
    FLS_INFO("C#", "Engine API bound (internal calls registration required)");
}

void CSharpBridge::reload() {
    auto files = m_loadedAssemblies;
    m_loadedAssemblies.clear();
    for (auto& f : files) loadAssembly(f);
}

} // namespace Feliss
