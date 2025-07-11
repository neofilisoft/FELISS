#include "FelissCore/FileSystem.h"
#include "FelissCore/Logger.h"
#include "FelissRenderer/FelissRenderer.h"

#include <configparser.hpp> // assume simple wrapper for INI (you can swap with inih or others)

using namespace FelissCore;
using namespace FelissRenderer;

int main() {
    Logger::Log("FelissEngine Launching...");

    ConfigParser config("../configuration.ini");
    std::string aa_mode = config.get("Graphics", "AntiAliasing", "TAA");

    Renderer renderer;
    AntiAliasingMode aa = RendererSettings::GetAAModeFromString(aa_mode);
    renderer.SetAntiAliasingMode(aa);

    for (int frame = 0; frame < 3; ++frame) {
        Logger::Log("Rendering frame " + std::to_string(frame));
    }

    Logger::Log("FelissEngine Shutdown.");
    return 0;
}
