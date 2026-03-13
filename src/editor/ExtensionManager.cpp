#include "editor/ExtensionManager.h"
#include "core/Engine.h"
#include "core/Logger.h"

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <dlfcn.h>
#  include <dirent.h>
#endif

#ifdef FELISS_EDITOR_BUILD
#  ifdef FELISS_HAS_IMGUI
#    include <imgui.h>
#  endif
#endif

#include <algorithm>
#include <filesystem>

namespace Feliss {

ExtensionManager::ExtensionManager(Engine& e) : m_engine(e) {}

ExtensionManager::~ExtensionManager() { unloadAll(); }

// ---- Platform helpers ----
void* ExtensionManager::loadLib(const std::string& path) {
#if defined(_WIN32)
    return LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}
void* ExtensionManager::getSymbol(void* lib, const std::string& sym) {
#if defined(_WIN32)
    return GetProcAddress(static_cast<HMODULE>(lib), sym.c_str());
#else
    return dlsym(lib, sym.c_str());
#endif
}
void ExtensionManager::closeLib(void* lib) {
    if (!lib) return;
#if defined(_WIN32)
    FreeLibrary(static_cast<HMODULE>(lib));
#else
    dlclose(lib);
#endif
}

FlsEngine ExtensionManager::engineHandle() {
    // We cast Engine* to FlsEngine (void*) for the C ABI
    return static_cast<FlsEngine>(static_cast<void*>(&m_engine));
}

bool ExtensionManager::load(const std::string& path) {
    void* lib = loadLib(path);
    if (!lib) {
        FLS_ERRORF("Extensions", "Failed to load: " << path
#ifndef _WIN32
            << " (" << dlerror() << ")"
#endif
        );
        return false;
    }

    // Resolve required symbols
    auto fnMeta     = reinterpret_cast<FlsExtMeta*(*)()>(getSymbol(lib, "fls_ext_meta"));
    auto fnInit     = reinterpret_cast<FlsStatus(*)(FlsEngine)>(getSymbol(lib, "fls_ext_init"));
    auto fnShutdown = reinterpret_cast<void(*)(FlsEngine)>(getSymbol(lib, "fls_ext_shutdown"));

    if (!fnMeta || !fnInit || !fnShutdown) {
        FLS_ERRORF("Extensions", "Missing required exports in: " << path);
        closeLib(lib);
        return false;
    }

    FlsExtMeta* meta = fnMeta();
    if (!meta) { closeLib(lib); return false; }

    // Prevent duplicate loads
    for (auto& e : m_extensions) {
        if (e.info.id == meta->id) {
            FLS_WARNF("Extensions", "Already loaded: " << meta->id);
            closeLib(lib);
            return false;
        }
    }

    LoadedExtension ext;
    ext.info.id          = meta->id;
    ext.info.name        = meta->name;
    ext.info.description = meta->description;
    ext.info.author      = meta->author;
    ext.info.version     = {meta->verMaj, meta->verMin, meta->verPat};
    ext.info.libraryPath = path;
    ext.info.enabled     = true;
    ext.libHandle        = lib;
    ext.fnMeta           = fnMeta;
    ext.fnInit           = fnInit;
    ext.fnShutdown       = fnShutdown;
    ext.fnUpdate = reinterpret_cast<void(*)(FlsEngine,float)>(getSymbol(lib,"fls_ext_update"));
    ext.fnImGui  = reinterpret_cast<void(*)(FlsEngine)>(getSymbol(lib,"fls_ext_imgui"));

    FlsStatus st = fnInit(engineHandle());
    if (st != FLS_OK) {
        FLS_ERRORF("Extensions", "fls_ext_init failed for: " << ext.info.id);
        closeLib(lib);
        return false;
    }

    m_extensions.push_back(std::move(ext));
    m_engine.events().emit(ExtensionLoadedEvent{ext.info.id});
    FLS_INFOF("Extensions", "Loaded: " << ext.info.name
        << " v" << meta->verMaj << "." << meta->verMin);
    return true;
}

bool ExtensionManager::unload(const std::string& id) {
    auto it = std::find_if(m_extensions.begin(), m_extensions.end(),
        [&](const LoadedExtension& e){ return e.info.id == id; });
    if (it == m_extensions.end()) return false;

    if (it->fnShutdown) it->fnShutdown(engineHandle());
    closeLib(it->libHandle);
    m_engine.events().emit(ExtensionUnloadedEvent{id});
    FLS_INFOF("Extensions", "Unloaded: " << id);
    m_extensions.erase(it);
    return true;
}

void ExtensionManager::unloadAll() {
    // shutdown in reverse load order
    for (int i = (int)m_extensions.size()-1; i >= 0; --i) {
        auto& e = m_extensions[i];
        if (e.fnShutdown) e.fnShutdown(engineHandle());
        closeLib(e.libHandle);
        FLS_INFOF("Extensions", "Unloaded: " << e.info.id);
    }
    m_extensions.clear();
}

void ExtensionManager::scanDirectory(const std::string& dir) {
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec)) return;

    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
#if defined(_WIN32)
        if (ext != ".dll") continue;
#elif defined(__APPLE__)
        if (ext != ".dylib") continue;
#else
        if (ext != ".so") continue;
#endif
        load(entry.path().string());
    }
}

void ExtensionManager::setEnabled(const std::string& id, bool en) {
    for (auto& e : m_extensions) if (e.info.id == id) { e.info.enabled = en; return; }
}
bool ExtensionManager::isEnabled(const std::string& id) const {
    for (auto& e : m_extensions) if (e.info.id == id) return e.info.enabled;
    return false;
}

void ExtensionManager::update(f32 dt) {
    for (auto& e : m_extensions)
        if (e.info.enabled && e.fnUpdate) e.fnUpdate(engineHandle(), dt);
}

void ExtensionManager::onImGui() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    // Per-extension ImGui panels
    for (auto& e : m_extensions)
        if (e.info.enabled && e.fnImGui) e.fnImGui(engineHandle());

    // Extension manager panel
    if (!ImGui::Begin("Extensions", &m_showPanel)) { ImGui::End(); return; }
    ImGui::Text("Loaded: %d", (int)m_extensions.size());
    ImGui::Separator();
    for (auto& e : m_extensions) {
        bool en = e.info.enabled;
        if (ImGui::Checkbox(e.info.name.c_str(), &en)) setEnabled(e.info.id, en);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s\n%s\nv%u.%u.%u\nAuthor: %s",
                e.info.id.c_str(), e.info.description.c_str(),
                e.info.version.major, e.info.version.minor, e.info.version.patch,
                e.info.author.c_str());
        ImGui::SameLine();
        std::string btnLabel = "Unload##" + e.info.id;
        if (ImGui::SmallButton(btnLabel.c_str())) { unload(e.info.id); break; }
    }
    ImGui::End();
#endif
}

const LoadedExtension* ExtensionManager::find(const std::string& id) const {
    for (auto& e : m_extensions) if (e.info.id == id) return &e;
    return nullptr;
}

} // namespace Feliss
