#pragma once
#include "feliss/Types.h"
#include "feliss/feliss_api.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace Feliss {

class Engine;

// =====================================================================
// ExtensionManager  — hot-load/unload .dll / .so / .dylib plugins
// Each plugin must export:
//   FlsExtMeta* fls_ext_meta()
//   FlsStatus   fls_ext_init(FlsEngine)
//   void        fls_ext_shutdown(FlsEngine)
// Optional:
//   void        fls_ext_update(FlsEngine, float dt)
//   void        fls_ext_imgui(FlsEngine)
// =====================================================================
struct LoadedExtension {
    ExtensionInfo info;
    void*         libHandle  = nullptr;

    // Function pointers
    FlsExtMeta* (*fnMeta)()                        = nullptr;
    FlsStatus   (*fnInit)(FlsEngine)               = nullptr;
    void        (*fnShutdown)(FlsEngine)            = nullptr;
    void        (*fnUpdate)(FlsEngine, float)       = nullptr;
    void        (*fnImGui)(FlsEngine)               = nullptr;
};

class ExtensionManager {
public:
    explicit ExtensionManager(Engine& engine);
    ~ExtensionManager();

    // Load a single .dll/.so
    bool       load(const std::string& path);
    bool       unload(const std::string& id);
    void       unloadAll();

    // Scan a directory for extension libraries
    void       scanDirectory(const std::string& dir);

    void       setEnabled(const std::string& id, bool en);
    bool       isEnabled(const std::string& id)  const;

    void       update(f32 dt);
    void       onImGui();           // called inside ImGui frame

    int        count()              const { return (int)m_extensions.size(); }
    const std::vector<LoadedExtension>& all() const { return m_extensions; }
    const LoadedExtension* find(const std::string& id) const;

private:
    void*  loadLib(const std::string& path);
    void*  getSymbol(void* lib, const std::string& sym);
    void   closeLib(void* lib);
    FlsEngine engineHandle();

    Engine&                         m_engine;
    std::vector<LoadedExtension>    m_extensions;
    bool m_showPanel = true;
};

} // namespace Feliss
