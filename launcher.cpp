#include <iostream>
#include "FelissCore/FileSystem.h"
#include "FelissCore/Logger.h"
#include "FelissRenderer/Renderer.h"
#include "FelissScene/SceneSystem.h"
#include "FelissPhysics/PhysicsWorld.h"
#include "FelissScript/LuaScripting.h"
#include "Runtime/AssetManager.h"
#include "Runtime/InputSystem.h"

using namespace FelissCore;
using namespace FelissRenderer;
using namespace FelissScene;
using namespace FelissPhysics;
using namespace FelissScript;

int main(int argc, char** argv) {
    Logger::Info("[FelissEngine] Launcher Started");

    
    if (!FileSystem::Exists("configuration.ini")) {
        Logger::Error("Missing configuration.ini");
        return -1;
    }

    
    Configuration config("configuration.ini");
    AntiAliasingMode aaMode = RendererSettings::GetAAModeFromString(config.Get("Renderer", "AA", "TAA"));

    Renderer renderer;
    renderer.SetAntiAliasingMode(aaMode);

    PhysicsWorld physics;
    SceneSystem scene;
    scene.Load("Assets/Scenes/demo.scene");

    
    bool running = true;
    float deltaTime = 1.0f / 60.0f;

    while (running) {
        InputSystem::Update();
        if (InputSystem::IsKeyPressed(KEY_ESC)) running = false;

        scene.Update(deltaTime);
        physics.Simulate(deltaTime);
        scene.DispatchToRenderAndPhysics(&renderer, &physics);
    }

    Logger::Info("[FelissEngine] Exiting");
    return 0;
}
