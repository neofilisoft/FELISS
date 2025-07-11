#include <string>
#include <imgui.h>
#include "FelissRenderer/FelissRenderer.h"
#include "FelissCore/FileSystem.h"
#include <configparser.hpp>

using namespace FelissRenderer;
using namespace FelissCore;

static const char* AA_MODE_NAMES[] = { "NONE", "FXAA", "SMAA", "TAA" };
static AntiAliasingMode TEMP_AA_MODE = AntiAliasingMode::TAA;

void DrawSettingsPanel(Renderer& renderer) {
    ImGui::Begin("Settings Panel");

    int currentAA = static_cast<int>(TEMP_AA_MODE);
    if (ImGui::Combo("Anti-Aliasing", &currentAA, AA_MODE_NAMES, IM_ARRAYSIZE(AA_MODE_NAMES))) {
        TEMP_AA_MODE = static_cast<AntiAliasingMode>(currentAA);
        renderer.SetAntiAliasingMode(TEMP_AA_MODE);

        // Save to ini file
        ConfigParser config("configuration.ini");
        config.set("Graphics", "AntiAliasing", RendererSettings::ToString(TEMP_AA_MODE));
        config.save();
        FelissCore::Logger::Log("[Editor] Updated AA Mode in config.ini");
    }

    ImGui::End();
}
