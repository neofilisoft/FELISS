#include <string>
#include <filesystem>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include "FelissRenderer/FelissRenderer.h"
#include <configparser.hpp>

namespace FelissEditor {

class HotReloadManager {
private:
    std::string configPath;
    std::filesystem::file_time_type lastModified;
    std::atomic<bool> running = true;
    std::thread watcher;

public:
    void Start(const std::string& path, FelissRenderer::Renderer* renderer) {
        configPath = path;
        if (!std::filesystem::exists(path)) return;
        lastModified = std::filesystem::last_write_time(path);

        watcher = std::thread([=]() {
            while (running.load()) {
                std::this_thread::sleep_for(std::chrono::seconds(2));

                auto nowMod = std::filesystem::last_write_time(path);
                if (nowMod != lastModified) {
                    lastModified = nowMod;
                    ConfigParser config(configPath);
                    auto modeStr = config.get("Graphics", "AntiAliasing", "TAA");
                    auto aa = FelissRenderer::RendererSettings::GetAAModeFromString(modeStr);
                    renderer->SetAntiAliasingMode(aa);
                    std::cout << "[HotReload] Updated AA Mode from config.ini to " << modeStr << std::endl;
                }
            }
        });
    }

    void Stop() {
        running = false;
        if (watcher.joinable()) watcher.join();
    }
};

} 
