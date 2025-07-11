#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace FelissScene {

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

struct Transform {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[3] = {0.0f, 0.0f, 0.0f};
    float scale[3]    = {1.0f, 1.0f, 1.0f};
};

struct IComponent {
    virtual ~IComponent() {}
};

struct Entity {
    EntityID id;
    std::string name;
    Transform transform;
    std::unordered_map<std::string, std::shared_ptr<IComponent>> components;

    template<typename T>
    void AddComponent(std::shared_ptr<T> comp) {
        components[typeid(T).name()] = comp;
    }

    template<typename T>
    T* GetComponent() {
        auto it = components.find(typeid(T).name());
        if (it != components.end())
            return static_cast<T*>(it->second.get());
        return nullptr;
    }
};

class Scene {
private:
    std::unordered_map<EntityID, Entity> entities;
    EntityID nextID = 1;

public:
    EntityID CreateEntity(const std::string& name) {
        Entity e;
        e.id = nextID++;
        e.name = name;
        entities[e.id] = e;
        std::cout << "[Scene] Created Entity: " << name << " ID=" << e.id << "\n";
        return e.id;
    }

    Entity* GetEntity(EntityID id) {
        auto it = entities.find(id);
        if (it != entities.end()) return &it->second;
        return nullptr;
    }

    void Clear() {
        entities.clear();
        nextID = 1;
    }
};

} 
