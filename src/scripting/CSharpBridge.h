#pragma once
#include "scripting/ScriptEngine.h"
#include <vector>
#include <string>

namespace Feliss {

// =====================================================================
// CSharpBridge — C# via Mono runtime (.NET 6+ or Mono JIT)
//
// C# BehaviourScript API:
//   public class MyScript : BehaviourScript {
//       public override void OnStart()              { ... }
//       public override void OnUpdate(float dt)     { ... }
//       public override void OnDestroy()            { ... }
//       public override void OnTriggerEnter(ulong id) { ... }
//
//       // Engine API:
//       Entity.Create("Name")
//       Transform.SetPosition(EntityId, new Vec3(0,1,0))
//       Log.Info("Hello from C#!")
//       Input.GetKey(KeyCode.Space)
//   }
// =====================================================================
class CSharpBridge : public IScriptBackend {
public:
    CSharpBridge();
    ~CSharpBridge() override;

    const char* name()       const override { return "C# (Mono)"; }
    bool        init()             override;
    void        shutdown()         override;
    void        update(f32 dt)     override;

    bool loadFile(const std::string& path)   override;
    bool loadString(const std::string& code,
                    const std::string& chunk) override;

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

    // Load a compiled .dll assembly
    bool loadAssembly(const std::string& dllPath);

private:
    // Opaque Mono handles (avoid exposing mono headers in this header)
    void* m_domain   = nullptr; // MonoDomain*
    void* m_assembly = nullptr; // MonoAssembly*
    void* m_image    = nullptr; // MonoImage*

    Engine* m_engine = nullptr;
    std::vector<std::string> m_loadedAssemblies;
};

} // namespace Feliss
