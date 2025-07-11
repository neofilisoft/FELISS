#pragma once

#include <imgui.h>

namespace EditorUI {

inline void RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {

        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "\xef\x88\x9b"); // Optional FontAwesome/Emoji fallback
        ImGui::SameLine();

        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("New Scene");
            ImGui::MenuItem("Open...");
            ImGui::MenuItem("Save");
            ImGui::MenuItem("Save As...");
            ImGui::Separator();
            ImGui::MenuItem("Exit");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo");
            ImGui::MenuItem("Redo");
            ImGui::Separator();
            ImGui::MenuItem("Preferences");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project")) {
            ImGui::MenuItem("Project Settings");
            ImGui::MenuItem("Build Project");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GameObject")) {
            ImGui::MenuItem("Create Empty");
            ImGui::MenuItem("3D Object");
            ImGui::MenuItem("Light");
            ImGui::MenuItem("Camera");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            ImGui::MenuItem("Scene Settings");
            ImGui::MenuItem("Bake Lighting");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Player Mode");
            ImGui::MenuItem("Gizmos");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Material Editor");
            ImGui::MenuItem("Animation Debugger");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Scene View");
            ImGui::MenuItem("Inspector");
            ImGui::MenuItem("Console");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("Documentation");
            ImGui::MenuItem("About FelissEngine");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

} 