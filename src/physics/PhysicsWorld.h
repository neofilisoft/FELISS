#pragma once
#include "feliss/Types.h"
#include <string>

namespace Feliss {

// =====================================================================
// PhysicsWorld — abstraction over PhysX / Jolt Physics
// Default: null physics (no-op) unless a physics extension is loaded
// =====================================================================
enum class PhysicsBackend : u32 { None=0, Jolt, PhysX };

struct PhysicsConfig {
    Vec3  gravity       = {0, -9.81f, 0};
    int   subSteps      = 1;
    bool  debugDraw     = false;
    PhysicsBackend backend = PhysicsBackend::None;
};

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    bool init(const PhysicsConfig& cfg = {});
    void shutdown();
    void step(f32 dt);

    // World properties
    void setGravity(Vec3 g);
    Vec3 getGravity() const { return m_gravity; }

    // Rigid body creation (returns opaque handle)
    void* createBody(EntityID owner, bool isStatic,
                     f32 mass, const Vec3& pos);
    void  destroyBody(void* handle);

    // Collider attachment
    void addBoxCollider(void* body, Vec3 halfExtents, f32 friction, f32 restitution);
    void addSphereCollider(void* body, f32 radius, f32 friction, f32 restitution);
    void addCapsuleCollider(void* body, f32 radius, f32 height, f32 friction, f32 restitution);

    // Raycast
    struct RayHit {
        bool     hit       = false;
        EntityID entity    = NULL_ENTITY;
        Vec3     point     = Vec3::zero();
        Vec3     normal    = Vec3::up();
        f32      distance  = 0.0f;
    };
    RayHit raycast(Vec3 origin, Vec3 direction, f32 maxDist, u32 layerMask = ~0u);

    PhysicsBackend activeBackend() const { return m_cfg.backend; }
    bool           isInitialized() const { return m_initialized; }

private:
    PhysicsConfig m_cfg;
    Vec3          m_gravity    = {0, -9.81f, 0};
    bool          m_initialized = false;
    void*         m_worldHandle = nullptr;  // Jolt / PhysX world ptr
};

} // namespace Feliss
