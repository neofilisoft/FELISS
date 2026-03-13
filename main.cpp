#include "core/Engine.h"
#include "core/Logger.h"

#ifdef FELISS_EDITOR_BUILD
#  include "editor/EditorApp.h"
#endif

int main(int argc, char** argv) {
    Feliss::EngineConfig cfg;
    cfg.title        = "Feliss Engine";
    cfg.windowWidth  = 1280;
    cfg.windowHeight = 720;
    cfg.renderAPI    = Feliss::RenderAPI::OpenGL;
    cfg.mode         = Feliss::EngineMode::Editor;
    cfg.enableLua    = true;
    cfg.enableCSharp = false;
    cfg.projectPath  = ".";
    cfg.logFile      = "feliss.log";

    // Parse simple CLI args
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--game")   cfg.mode = Feliss::EngineMode::Game;
        else if (a == "--server") cfg.mode = Feliss::EngineMode::Server;
        else if (a == "--vulkan") cfg.renderAPI = Feliss::RenderAPI::Vulkan;
        else if (a == "--dx11")   cfg.renderAPI = Feliss::RenderAPI::DirectX11;
        else if (a == "--dx12")   cfg.renderAPI = Feliss::RenderAPI::DirectX12;
        else if (a == "--metal")  cfg.renderAPI = Feliss::RenderAPI::Metal;
        else if (a == "--csharp") cfg.enableCSharp = true;
    }

    Feliss::Engine engine(cfg);
    if (!engine.init()) {
        FLS_FATAL("Main", "Engine init failed — aborting");
        return 1;
    }

#ifdef FELISS_EDITOR_BUILD
    Feliss::EditorApp editor(engine);
    editor.init();
    engine.setRenderCallback([&]{ editor.render(); });
#endif

    engine.run();
    return 0;
}
