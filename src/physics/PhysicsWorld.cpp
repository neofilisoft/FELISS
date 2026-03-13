#include "physics/PhysicsWorld.h"
#include "core/Logger.h"

namespace Feliss {

PhysicsWorld::PhysicsWorld()  = default;
PhysicsWorld::~PhysicsWorld() { shutdown(); }

bool PhysicsWorld::init(const PhysicsConfig& cfg) {
    m_cfg = cfg;
    m_gravity = cfg.gravity;

    switch (cfg.backend) {
        case PhysicsBackend::Jolt:
            FLS_INFO("Physics", "Jolt backend selected (stub — link Jolt library)");
            break;
        case PhysicsBackend::PhysX:
            FLS_INFO("Physics", "PhysX backend selected (stub — link PhysX SDK)");
            break;
        default:
            FLS_INFO("Physics", "No physics backend — simulation disabled");
            break;
    }
    m_initialized = true;
    return true;
}

void PhysicsWorld::shutdown() {
    if (!m_initialized) return;
    m_worldHandle = nullptr;
    m_initialized = false;
    FLS_INFO("Physics", "PhysicsWorld shutdown");
}

void PhysicsWorld::step(f32 dt) {
    if (!m_initialized || m_cfg.backend == PhysicsBackend::None) return;
    // TODO: call Jolt/PhysX step
}

void PhysicsWorld::setGravity(Vec3 g) {
    m_gravity = g;
    // TODO: propagate to backend
}

void* PhysicsWorld::createBody(EntityID, bool, f32, const Vec3&) { return nullptr; }
void  PhysicsWorld::destroyBody(void*) {}
void  PhysicsWorld::addBoxCollider(void*,Vec3,f32,f32) {}
void  PhysicsWorld::addSphereCollider(void*,f32,f32,f32) {}
void  PhysicsWorld::addCapsuleCollider(void*,f32,f32,f32,f32) {}

PhysicsWorld::RayHit PhysicsWorld::raycast(Vec3, Vec3, f32, u32) {
    return {}; // stub
}

} // namespace Feliss
