
#include "FelissCore/Logger.h"
#include "FelissCore/FileSystem.h"
#include "FelissRenderer/FelissRenderer.h"
#include "SceneSystem.h"
#include "SceneSerializer.h"

class PhysicsWorld {
public:
    void AddBody(EntityID id, const float* pos, float mass, bool isStatic) {
        FelissCore::Logger::Info("[Physics] Added Body ID: " + std::to_string(id) +
            " Mass: " + std::to_string(mass) + (isStatic ? " [Static]" : ""));
    }
};

namespace FelissRenderer {
void Renderer::RegisterMesh(EntityID id, const std::string& path) {
    FelissCore::Logger::Info("[Render] Registered Mesh ID: " + std::to_string(id) + " -> " + path);
}
} 

int main() {
    using namespace FelissCore;
    using namespace FelissRenderer;

    Logger::Log("FelissEngine Scene Test...");

    if (!FileSystem::Exists("../test_scene.json")) {
        Logger::Error("Missing scene file: ../test_scene.json");
        return -1;
    }

    Scene scene;
    SceneSerializer::LoadScene(scene, "../test_scene.json");

    Renderer renderer;
    PhysicsWorld physics;

    scene.DispatchToRenderAndPhysics(&renderer, &physics);

    Logger::Log("Scene load + dispatch completed.");
    return 0;
}
