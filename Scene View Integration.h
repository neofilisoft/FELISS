#pragma once

#include "SceneSystem.h"
#include <memory>

class SceneView {
private:
    Scene* scene;

public:
    SceneView() : scene(nullptr) {}

    void SetScene(Scene* s) { scene = s; }

    void Render() {
        if (!scene) return;

        for (const auto& [id, ent] : scene->GetAllEntities()) {
            // Mocked scene rendering: show transform and name
            printf("[SceneView] Entity %llu (%s)\n", id, ent.name.c_str());
            printf("  Pos: %.2f, %.2f, %.2f\n",
                ent.transform.position[0],
                ent.transform.position[1],
                ent.transform.position[2]);

            if (ent.mesh)
                printf("  Mesh: %s\n", ent.mesh->meshPath.c_str());

            if (ent.rigidbody)
                printf("  RigidBody: mass=%.2f static=%d\n",
                    ent.rigidbody->mass,
                    ent.rigidbody->isStatic);
        }
    }
};
