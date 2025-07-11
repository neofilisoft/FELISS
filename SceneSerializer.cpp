#pragma once

#include "SceneSystem.h"
#include "Components/MeshComponent.h"
#include "Components/RigidBodyComponent.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace FelissScene {

class SceneSerializer {
public:
    static void SaveScene(const Scene& scene, const std::string& filepath) {
        json jscene;

        for (const auto& [id, entity] : scene.GetAllEntities()) {
            json jent;
            jent["id"] = entity.id;
            jent["name"] = entity.name;
            jent["position"] = { entity.transform.position[0], entity.transform.position[1], entity.transform.position[2] };
            jent["rotation"] = { entity.transform.rotation[0], entity.transform.rotation[1], entity.transform.rotation[2] };
            jent["scale"]    = { entity.transform.scale[0], entity.transform.scale[1], entity.transform.scale[2] };

            if (auto* mesh = entity.GetComponent<MeshComponent>()) {
                jent["mesh"] = mesh->meshPath;
            }

            if (auto* rb = entity.GetComponent<RigidBodyComponent>()) {
                jent["rigidbody"] = {
                    {"mass", rb->mass},
                    {"isStatic", rb->isStatic}
                };
            }

            jscene["entities"].push_back(jent);
        }

        std::ofstream out(filepath);
        out << jscene.dump(4);
    }

    static void LoadScene(Scene& scene, const std::string& filepath) {
        std::ifstream in(filepath);
        if (!in.is_open()) return;

        json jscene;
        in >> jscene;

        scene.Clear();
        for (const auto& jent : jscene["entities"]) {
            EntityID id = scene.CreateEntity(jent["name"]);
            Entity* e = scene.GetEntity(id);
            auto pos = jent["position"];
            auto rot = jent["rotation"];
            auto scl = jent["scale"];

            for (int i = 0; i < 3; ++i) {
                e->transform.position[i] = pos[i];
                e->transform.rotation[i] = rot[i];
                e->transform.scale[i]    = scl[i];
            }

            if (jent.contains("mesh")) {
                auto mesh = std::make_shared<MeshComponent>();
                mesh->meshPath = jent["mesh"];
                e->AddComponent(mesh);
            }

            if (jent.contains("rigidbody")) {
                auto rb = std::make_shared<RigidBodyComponent>();
                rb->mass = jent["rigidbody"]["mass"];
                rb->isStatic = jent["rigidbody"]["isStatic"];
                e->AddComponent(rb);
            }
        }
    }
};

}