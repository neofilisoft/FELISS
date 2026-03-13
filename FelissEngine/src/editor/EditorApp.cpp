#include "editor/EditorApp.h"
#include "core/Engine.h"
#include "ecs/World.h"
#include "ecs/Component.h"
#include "renderer/RenderPipeline.h"
#include "core/Logger.h"

#ifdef FELISS_EDITOR_BUILD
#  ifdef FELISS_HAS_IMGUI
#    include <imgui.h>
#    include <imgui_internal.h>
#  endif
#endif

#include <cstring>
#include <cstdio>
#include <filesystem>

namespace Feliss {

EditorApp::EditorApp(Engine& e) : m_engine(e) {}

bool EditorApp::init() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    FLS_INFO("Editor","EditorApp initialized");
    return true;
#else
    return true;
#endif
}

void EditorApp::shutdown() {}

void EditorApp::render() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    drawMainMenuBar();
    drawDockspace();
    drawToolbar();
    drawHierarchy();
    drawInspector();
    drawSceneView();
    drawAssetBrowser();
    drawConsole();
    drawRenderPipelineEditor();
#endif
}

// ---- Main menu bar ----
void EditorApp::drawMainMenuBar() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene",     "Ctrl+N")) {}
        if (ImGui::MenuItem("Open Scene",    "Ctrl+O")) {}
        if (ImGui::MenuItem("Save Scene",    "Ctrl+S")) {
            m_engine.world().saveToFile("scene.bdk");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit",          "Alt+F4")) m_engine.stop();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo","Ctrl+Z")) {}
        if (ImGui::MenuItem("Redo","Ctrl+Y")) {}
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Entity")) {
        if (ImGui::MenuItem("Create Empty")) {
            EntityID id = m_engine.world().createEntity("Entity");
            m_selected = id;
        }
        if (ImGui::MenuItem("Create Camera")) {
            EntityID id = m_engine.world().createEntity("Camera");
            m_engine.world().addComponent<CameraComponent>(id);
            m_selected = id;
        }
        if (ImGui::MenuItem("Create Directional Light")) {
            EntityID id = m_engine.world().createEntity("DirectionalLight");
            auto& l = m_engine.world().addComponent<LightComponent>(id);
            l.type = LightComponent::LightType::Directional;
            m_selected = id;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Grid", nullptr, &m_showGrid);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Scripts")) {
        if (ImGui::MenuItem("Reload All")) m_engine.scripts().reloadAll();
        ImGui::EndMenu();
    }

    // Runtime info on right side
    char info[64];
    snprintf(info, sizeof(info), "%.1f fps | %s",
        m_engine.fps(),
        RenderAPIToString(m_engine.renderer().api()));
    float infoW = ImGui::CalcTextSize(info).x + 16.f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - infoW);
    ImGui::TextDisabled("%s", info);

    ImGui::EndMainMenuBar();
#endif
}

// ---- Dockspace ----
void EditorApp::drawDockspace() {
#ifdef FELISS_HAS_IMGUI
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("##DockRoot", nullptr, flags);
    ImGui::PopStyleVar();
    ImGui::DockSpace(ImGui::GetID("MainDock"),
        ImVec2(0,0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
#endif
}

// ---- Toolbar ----
void EditorApp::drawToolbar() {
#ifdef FELISS_HAS_IMGUI
    ImGui::Begin("##Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove);

    // Play / Pause / Stop
    if (m_playState == PlayState::Stopped) {
        if (ImGui::Button("  Play  ")) {
            m_playState = PlayState::Playing;
            FLS_INFO("Editor","Play");
        }
    } else {
        if (ImGui::Button("  Stop  ")) {
            m_playState = PlayState::Stopped;
            FLS_INFO("Editor","Stop");
        }
        ImGui::SameLine();
        const char* pauseLabel = (m_playState==PlayState::Paused) ? "Resume" : " Pause ";
        if (ImGui::Button(pauseLabel))
            m_playState = (m_playState==PlayState::Playing) ? PlayState::Paused : PlayState::Playing;
    }

    ImGui::SameLine(0, 20);

    // Gizmo tools
    const char* tools[] = {"Select","Move","Rotate","Scale"};
    for (int i = 0; i < 4; ++i) {
        bool active = (m_tool == static_cast<GizmoTool>(i));
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::Button(tools[i])) m_tool = static_cast<GizmoTool>(i);
        if (active) ImGui::PopStyleColor();
        ImGui::SameLine();
    }
    ImGui::End();
#endif
}

// ---- Hierarchy ----
void EditorApp::drawHierarchy() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Hierarchy")) { ImGui::End(); return; }

    if (ImGui::Button("+ Entity")) {
        m_selected = m_engine.world().createEntity("Entity");
    }

    ImGui::Separator();

    auto roots = m_engine.world().getRoots();
    for (EntityID id : roots) drawEntityNode(id);

    // Deselect on blank click
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
        m_selected = NULL_ENTITY;

    ImGui::End();
#endif
}

void EditorApp::drawEntityNode(EntityID id) {
#ifdef FELISS_HAS_IMGUI
    World& w = m_engine.world();
    std::string name = w.getName(id);
    bool hasChildren = !w.getChildren(id).empty();

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasChildren)  flags |= ImGuiTreeNodeFlags_Leaf;
    if (id == m_selected) flags |= ImGuiTreeNodeFlags_Selected;

    // Inline rename
    bool open = false;
    if (m_renaming && m_renamingID == id) {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##rename", m_renameBuffer, sizeof(m_renameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            w.setName(id, m_renameBuffer);
            m_renaming = false;
        }
        if (!ImGui::IsItemActive() && !ImGui::IsItemFocused()) m_renaming = false;
    } else {
        open = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s", name.c_str());
        if (ImGui::IsItemClicked())      m_selected = id;
        if (ImGui::IsItemDoubleClicked()){ m_renaming=true; m_renamingID=id; std::strncpy(m_renameBuffer,name.c_str(),255); }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Create Child")) {
                EntityID child = w.createEntity("Child");
                w.setParent(child, id);
            }
            if (ImGui::MenuItem("Duplicate")) {
                EntityID dup = w.createEntity(name + "_copy");
                w.setParent(dup, w.getParent(id));
                m_selected = dup;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete")) {
                w.destroyEntity(id);
                if (m_selected == id) m_selected = NULL_ENTITY;
                ImGui::EndPopup();
                if (open) ImGui::TreePop();
                return;
            }
            ImGui::EndPopup();
        }
    }

    if (open) {
        for (EntityID child : w.getChildren(id)) drawEntityNode(child);
        ImGui::TreePop();
    }
#endif
}

// ---- Inspector ----
void EditorApp::drawInspector() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Inspector")) { ImGui::End(); return; }
    if (m_selected == NULL_ENTITY || !m_engine.world().isValid(m_selected)) {
        ImGui::TextDisabled("No entity selected");
        ImGui::End(); return;
    }
    drawComponentEditors(m_selected);
    ImGui::End();
#endif
}

void EditorApp::drawComponentEditors(EntityID id) {
#ifdef FELISS_HAS_IMGUI
    World& w = m_engine.world();

    // ---- Tag ----
    if (auto* c = w.getComponent<TagComponent>(id)) {
        ImGui::Checkbox("##active", &c->active);
        ImGui::SameLine();
        char buf[256];
        std::strncpy(buf, c->name.c_str(), 255);
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##name", buf, 255))
            w.setName(id, buf);
        ImGui::Text("Tag:"); ImGui::SameLine();
        char tbuf[128];
        std::strncpy(tbuf, c->tag.c_str(), 127);
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##tag", tbuf, 127))
            w.setTag(id, tbuf);
        ImGui::Separator();
    }

    // ---- Transform ----
    if (auto* t = w.getComponent<TransformComponent>(id)) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            float p[3] = {t->position.x, t->position.y, t->position.z};
            float s[3] = {t->scale.x,    t->scale.y,    t->scale.z};
            if (ImGui::DragFloat3("Position", p, 0.01f)) {
                t->position = {p[0],p[1],p[2]}; t->dirty = true;
            }
            // Euler display (simplified)
            float e[3] = {t->rotation.x, t->rotation.y, t->rotation.z};
            if (ImGui::DragFloat3("Rotation", e, 0.5f)) {
                t->rotation = {e[0],e[1],e[2],t->rotation.w}; t->dirty = true;
            }
            if (ImGui::DragFloat3("Scale", s, 0.01f)) {
                t->scale = {s[0],s[1],s[2]}; t->dirty = true;
            }
        }
    }

    // ---- MeshRenderer ----
    if (auto* mr = w.getComponent<MeshRendererComponent>(id)) {
        if (ImGui::CollapsingHeader("Mesh Renderer")) {
            ImGui::Checkbox("Enabled##mr", &mr->enabled);
            ImGui::Checkbox("Cast Shadow",    &mr->castShadow);
            ImGui::Checkbox("Receive Shadow", &mr->receiveShadow);
            ImGui::Text("Mesh ID: %llu",     (unsigned long long)mr->meshID);
            ImGui::Text("Material ID: %llu", (unsigned long long)mr->materialID);
        }
    }

    // ---- Sprite ----
    if (auto* sp = w.getComponent<SpriteComponent>(id)) {
        if (ImGui::CollapsingHeader("Sprite")) {
            ImGui::Checkbox("Enabled##sp", &sp->enabled);
            float col[4] = {sp->tint.r, sp->tint.g, sp->tint.b, sp->tint.a};
            if (ImGui::ColorEdit4("Tint", col))
                sp->tint = {col[0],col[1],col[2],col[3]};
            float sz[2] = {sp->size.x, sp->size.y};
            if (ImGui::DragFloat2("Size", sz, 0.01f))
                sp->size = {sz[0], sz[1]};
            ImGui::DragInt("Sort Order", &sp->sortOrder);
            ImGui::Checkbox("Flip X", &sp->flipX); ImGui::SameLine();
            ImGui::Checkbox("Flip Y", &sp->flipY);
            ImGui::Checkbox("Billboard (2.5D)", &sp->billboard);
            ImGui::Checkbox("Pixel Snap",       &sp->pixelSnap);
        }
    }

    // ---- Camera ----
    if (auto* cam = w.getComponent<CameraComponent>(id)) {
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::Checkbox("Is Main", &cam->isMain);
            int proj = (int)cam->projection;
            if (ImGui::Combo("Projection", &proj, "Perspective\0Orthographic\0"))
                cam->projection = (CameraComponent::Projection)proj;
            if (cam->projection == CameraComponent::Projection::Perspective)
                ImGui::DragFloat("FOV", &cam->fov, 0.5f, 1.f, 180.f);
            else
                ImGui::DragFloat("Ortho Size", &cam->orthoSize, 0.1f, 0.1f, 100.f);
            ImGui::DragFloat("Near Clip", &cam->nearClip, 0.001f, 0.001f, cam->farClip);
            ImGui::DragFloat("Far Clip",  &cam->farClip,  1.f, cam->nearClip, 10000.f);
            float cc[4] = {cam->clearColor.r,cam->clearColor.g,cam->clearColor.b,cam->clearColor.a};
            if (ImGui::ColorEdit4("Clear Color", cc))
                cam->clearColor = {cc[0],cc[1],cc[2],cc[3]};
        }
    }

    // ---- Light ----
    if (auto* l = w.getComponent<LightComponent>(id)) {
        if (ImGui::CollapsingHeader("Light")) {
            ImGui::Checkbox("Enabled##lt", &l->enabled);
            int lt = (int)l->type;
            if (ImGui::Combo("Type", &lt, "Directional\0Point\0Spot\0Area\0"))
                l->type = (LightComponent::LightType)lt;
            float lc[4] = {l->color.r,l->color.g,l->color.b,l->color.a};
            if (ImGui::ColorEdit4("Color##lt", lc))
                l->color = {lc[0],lc[1],lc[2],lc[3]};
            ImGui::DragFloat("Intensity", &l->intensity, 0.05f, 0.f, 100.f);
            if (l->type != LightComponent::LightType::Directional)
                ImGui::DragFloat("Range", &l->range, 0.1f, 0.f, 1000.f);
            if (l->type == LightComponent::LightType::Spot) {
                ImGui::DragFloat("Spot Inner", &l->spotInner, 0.5f, 0.f, l->spotOuter);
                ImGui::DragFloat("Spot Outer", &l->spotOuter, 0.5f, l->spotInner, 90.f);
            }
            ImGui::Checkbox("Cast Shadow", &l->castShadow);
        }
    }

    // ---- RigidBody ----
    if (auto* rb = w.getComponent<RigidBodyComponent>(id)) {
        if (ImGui::CollapsingHeader("Rigid Body")) {
            ImGui::Checkbox("Enabled##rb", &rb->enabled);
            int bt = (int)rb->type;
            if (ImGui::Combo("Body Type", &bt, "Static\0Kinematic\0Dynamic\0"))
                rb->type = (RigidBodyComponent::BodyType)bt;
            ImGui::DragFloat("Mass", &rb->mass, 0.1f, 0.f, 10000.f);
            ImGui::DragFloat("Drag", &rb->drag, 0.01f, 0.f, 10.f);
            ImGui::Checkbox("Use Gravity", &rb->useGravity);
        }
    }

    // ---- Script ----
    if (auto* sc = w.getComponent<ScriptComponent>(id)) {
        if (ImGui::CollapsingHeader("Scripts")) {
            for (auto& inst : sc->scripts) {
                ImGui::BulletText("[%s] %s", inst.isLua?"Lua":"C#", inst.className.c_str());
                ImGui::SameLine();
                ImGui::Checkbox(("##en" + inst.className).c_str(), &inst.enabled);
            }
            if (ImGui::Button("+ Add Script")) {}
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Add Component ----
    if (ImGui::Button("Add Component", ImVec2(-1, 0))) ImGui::OpenPopup("AddComp");
    if (ImGui::BeginPopup("AddComp")) {
        if (ImGui::MenuItem("Mesh Renderer")   && !w.hasComponent<MeshRendererComponent>(id))
            w.addComponent<MeshRendererComponent>(id);
        if (ImGui::MenuItem("Sprite")          && !w.hasComponent<SpriteComponent>(id))
            w.addComponent<SpriteComponent>(id);
        if (ImGui::MenuItem("Camera")          && !w.hasComponent<CameraComponent>(id))
            w.addComponent<CameraComponent>(id);
        if (ImGui::MenuItem("Light")           && !w.hasComponent<LightComponent>(id))
            w.addComponent<LightComponent>(id);
        if (ImGui::MenuItem("Rigid Body")      && !w.hasComponent<RigidBodyComponent>(id))
            w.addComponent<RigidBodyComponent>(id);
        if (ImGui::MenuItem("Box Collider")    && !w.hasComponent<BoxColliderComponent>(id))
            w.addComponent<BoxColliderComponent>(id);
        if (ImGui::MenuItem("Sphere Collider") && !w.hasComponent<SphereColliderComponent>(id))
            w.addComponent<SphereColliderComponent>(id);
        if (ImGui::MenuItem("Audio Source")    && !w.hasComponent<AudioSourceComponent>(id))
            w.addComponent<AudioSourceComponent>(id);
        if (ImGui::MenuItem("Animator")        && !w.hasComponent<AnimatorComponent>(id))
            w.addComponent<AnimatorComponent>(id);
        if (ImGui::MenuItem("Particle System") && !w.hasComponent<ParticleSystemComponent>(id))
            w.addComponent<ParticleSystemComponent>(id);
        if (ImGui::MenuItem("Script")          && !w.hasComponent<ScriptComponent>(id))
            w.addComponent<ScriptComponent>(id);
        ImGui::EndPopup();
    }
#endif
}

// ---- Scene View ----
void EditorApp::drawSceneView() {
#ifdef FELISS_HAS_IMGUI
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    if (!ImGui::Begin("Scene View")) { ImGui::PopStyleVar(); ImGui::End(); return; }
    ImVec2 sz = ImGui::GetContentRegionAvail();
    if (sz.x > 1 && sz.y > 1) {
        // Render target texture handle (placeholder until framebuffer is wired)
        ImGui::Image(nullptr, sz);
        // Overlay info
        ImGui::SetCursorPos(ImVec2(8,24));
        ImGui::TextDisabled("Scene  %dx%d  |  %s",
            (int)sz.x, (int)sz.y,
            PipelineModeToString(m_engine.renderer().getPipelineMode()));
    }
    ImGui::End();
    ImGui::PopStyleVar();
#endif
}

// ---- Asset Browser ----
void EditorApp::drawAssetBrowser() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Asset Browser")) { ImGui::End(); return; }
    // Left: directory tree
    ImGui::BeginChild("DirTree", ImVec2(180,0), true);
    std::error_code ec;
    if (std::filesystem::is_directory(m_assetPath, ec)) {
        for (auto& e : std::filesystem::directory_iterator(m_assetPath, ec)) {
            if (!e.is_directory()) continue;
            std::string name = e.path().filename().string();
            if (ImGui::Selectable(name.c_str())) m_assetPath = e.path().string();
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    // Right: icon grid
    ImGui::BeginChild("Assets", ImVec2(0,0), false);
    float panelW = ImGui::GetContentRegionAvail().x;
    int cols = std::max(1, (int)(panelW / (m_iconSize + 8)));
    int col  = 0;
    if (std::filesystem::is_directory(m_assetPath, ec)) {
        for (auto& e : std::filesystem::directory_iterator(m_assetPath, ec)) {
            std::string name = e.path().filename().string();
            ImGui::BeginGroup();
            ImGui::Button(name.substr(0,10).c_str(), ImVec2(m_iconSize, m_iconSize));
            ImGui::TextWrapped("%s", name.c_str());
            ImGui::EndGroup();
            if (++col < cols) ImGui::SameLine();
            else col = 0;
        }
    }
    ImGui::EndChild();
    ImGui::End();
#endif
}

// ---- Console ----
void EditorApp::drawConsole() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Console")) { ImGui::End(); return; }

    // Filter checkboxes
    const char* labels[] = {"TRC","DBG","INF","WRN","ERR","FTL"};
    for (int i = 0; i < 6; ++i) {
        ImGui::Checkbox(labels[i], &m_showLog[i]);
        if (i < 5) ImGui::SameLine();
    }
    ImGui::SameLine(0, 20);
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Filter", m_consoleFilter, sizeof(m_consoleFilter));
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) Logger::get().clearEntries();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_consoleAutoScroll);
    ImGui::Separator();

    ImGui::BeginChild("LogScroll", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (auto& e : Logger::get().entries()) {
        int lvl = (int)e.level;
        if (lvl < 0 || lvl > 5 || !m_showLog[lvl]) continue;
        if (m_consoleFilter[0] && e.message.find(m_consoleFilter) == std::string::npos) continue;
        ImVec4 col = ImVec4(1,1,1,1);
        switch(e.level) {
            case LogLevel::Trace:   col = ImVec4(.5f,.5f,.5f,1); break;
            case LogLevel::Debug:   col = ImVec4(.4f,.8f,.9f,1); break;
            case LogLevel::Info:    col = ImVec4(.8f,.9f,.8f,1); break;
            case LogLevel::Warning: col = ImVec4(1.f,.9f,.2f,1); break;
            case LogLevel::Error:   col = ImVec4(1.f,.4f,.4f,1); break;
            case LogLevel::Fatal:   col = ImVec4(.9f,.2f,.9f,1); break;
            default: break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(e.message.c_str());
        ImGui::PopStyleColor();
    }
    if (m_consoleAutoScroll) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::End();
#endif
}

// ---- Render Pipeline Editor ----
void EditorApp::drawRenderPipelineEditor() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Render Pipeline")) { ImGui::End(); return; }

    RenderPipeline& rp = m_engine.renderer();
    ImGui::Text("API: %s", RenderAPIToString(rp.api()));
    ImGui::Text("Resolution: %dx%d", rp.width(), rp.height());
    ImGui::Separator();

    // Pipeline mode
    int mode = (int)rp.getPipelineMode();
    if (ImGui::Combo("Mode", &mode, "PBR 3D\0HD-2D / 2.5D\0Pixel Art 2D\0Toon\0Custom\0"))
        rp.setPipelineMode((PipelineMode)mode);

    ImGui::Separator();
    ImGui::Text("Render Passes:");
    for (auto& pass : rp.passes()) {
        bool en = pass.enabled;
        if (ImGui::Checkbox(pass.name.c_str(), &en))
            rp.setPassEnabled(pass.name, en);
    }

    ImGui::Separator();
    ImGui::Text("Post Effects:");
    for (auto& fx : rp.postEffects()) {
        bool en = fx.enabled;
        if (ImGui::Checkbox(fx.name.c_str(), &en))
            rp.setPostEffectEnabled(fx.name, en);
    }

    ImGui::Separator();
    const auto& st = rp.stats();
    ImGui::Text("Draw calls: %u", st.drawCalls);
    ImGui::Text("Sprites:    %u", st.spriteCalls);
    ImGui::Text("Triangles:  %u", st.triangles);
    ImGui::Text("Lights:     %u", st.lightCount);
    ImGui::Text("GPU ms:     %.2f", st.gpuMs);

    ImGui::End();
#endif
}

} // namespace Feliss
