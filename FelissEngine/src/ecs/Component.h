#pragma once
#include "feliss/Types.h"
#include <string>
#include <vector>
#include <typeindex>

namespace Feliss {

// =====================================================================
// ComponentBase — all components inherit from this
// =====================================================================
struct ComponentBase {
    bool enabled = true;
    virtual ~ComponentBase() = default;
    virtual const char* typeName() const = 0;
};

#define FLS_COMPONENT(T) \
    static const char* staticTypeName() { return #T; } \
    const char* typeName() const override { return #T; }

// =====================================================================
// Built-in components
// =====================================================================

struct TagComponent : ComponentBase {
    FLS_COMPONENT(TagComponent)
    std::string name;
    std::string tag;
    u32         layer  = 0;
    bool        active = true;
    explicit TagComponent(std::string n = "Entity") : name(std::move(n)) {}
};

struct TransformComponent : ComponentBase {
    FLS_COMPONENT(TransformComponent)
    Vec3 position = Vec3::zero();
    Quat rotation = Quat::identity();
    Vec3 scale    = Vec3::one();
    EntityID parentID = NULL_ENTITY;
    bool dirty = true;
    // Cached world matrix (updated by TransformSystem)
    Mat4 worldMatrix;
};

struct MeshRendererComponent : ComponentBase {
    FLS_COMPONENT(MeshRendererComponent)
    AssetID meshID     = NULL_ASSET;
    AssetID materialID = NULL_ASSET;
    bool    castShadow    = true;
    bool    receiveShadow = true;
    u32     renderLayer   = 1;
};

struct SpriteComponent : ComponentBase {
    FLS_COMPONENT(SpriteComponent)
    AssetID textureID = NULL_ASSET;
    Color   tint      = Color::white();
    Vec2    size      = {1,1};
    Vec2    pivot     = {0.5f,0.5f};
    BlendMode blendMode = BlendMode::Alpha;
    int     sortOrder  = 0;
    bool    flipX = false, flipY = false;
    bool    billboard = false;   // for 2.5D
    bool    pixelSnap = false;   // for pixel art
};

struct CameraComponent : ComponentBase {
    FLS_COMPONENT(CameraComponent)
    enum class Projection { Perspective, Orthographic };
    Projection projection = Projection::Perspective;
    f32 fov        = 60.0f;
    f32 orthoSize  = 10.0f;
    f32 nearClip   = 0.1f;
    f32 farClip    = 1000.0f;
    f32 aspectRatio = 16.0f/9.0f;
    Color clearColor = Color::black();
    bool  isMain   = false;
    u32   cullingMask = 0xFFFFFFFF;
    AssetID renderTargetID = NULL_ASSET;  // 0 = back buffer
};

struct LightComponent : ComponentBase {
    FLS_COMPONENT(LightComponent)
    enum class LightType { Directional=0, Point, Spot, Area };
    LightType type      = LightType::Point;
    Color     color     = Color::white();
    f32       intensity = 1.0f;
    f32       range     = 10.0f;
    f32       spotInner = 15.0f;  // degrees
    f32       spotOuter = 30.0f;
    bool      castShadow = true;
    f32       shadowBias = 0.001f;
    i32       shadowResolution = 1024;
};

struct RigidBodyComponent : ComponentBase {
    FLS_COMPONENT(RigidBodyComponent)
    enum class BodyType { Static=0, Kinematic, Dynamic };
    BodyType type   = BodyType::Dynamic;
    f32      mass   = 1.0f;
    f32      drag   = 0.0f;
    f32      angularDrag = 0.05f;
    bool     useGravity  = true;
    bool     isTrigger   = false;
    Vec3     linearVelocity  = Vec3::zero();
    Vec3     angularVelocity = Vec3::zero();
    u32      physicsLayer = 1;
    void*    bodyHandle   = nullptr;  // opaque physics body
};

struct BoxColliderComponent : ComponentBase {
    FLS_COMPONENT(BoxColliderComponent)
    Vec3 center  = Vec3::zero();
    Vec3 size    = Vec3::one();
    f32  friction    = 0.5f;
    f32  restitution = 0.0f;
    bool isTrigger   = false;
};

struct SphereColliderComponent : ComponentBase {
    FLS_COMPONENT(SphereColliderComponent)
    Vec3 center  = Vec3::zero();
    f32  radius  = 0.5f;
    f32  friction    = 0.5f;
    f32  restitution = 0.0f;
    bool isTrigger   = false;
};

struct AudioSourceComponent : ComponentBase {
    FLS_COMPONENT(AudioSourceComponent)
    AssetID clipID      = NULL_ASSET;
    f32     volume      = 1.0f;
    f32     pitch       = 1.0f;
    f32     minDist     = 1.0f;
    f32     maxDist     = 100.0f;
    bool    loop        = false;
    bool    playOnAwake = false;
    bool    spatial3D   = true;
    void*   sourceHandle = nullptr;
};

struct AudioListenerComponent : ComponentBase {
    FLS_COMPONENT(AudioListenerComponent)
    bool isMain = true;
};

struct AnimatorComponent : ComponentBase {
    FLS_COMPONENT(AnimatorComponent)
    AssetID controllerID = NULL_ASSET;
    f32     speed        = 1.0f;
    std::string currentState;
    bool    playing = false;
    bool    loop    = true;
};

struct ScriptComponent : ComponentBase {
    FLS_COMPONENT(ScriptComponent)
    struct Instance {
        std::string className;
        bool        isLua    = false;   // false = C#, true = Lua
        bool        enabled  = true;
        void*       handle   = nullptr; // managed object / lua ref
    };
    std::vector<Instance> scripts;
};

struct ParticleSystemComponent : ComponentBase {
    FLS_COMPONENT(ParticleSystemComponent)
    u32   maxParticles = 1000;
    f32   emitRate     = 50.0f;
    f32   lifetime     = 2.0f;
    Vec3  startVelocity = {0,1,0};
    Vec3  gravity       = {0,-9.81f,0};
    Color startColor    = Color::white();
    Color endColor      = Color::clear();
    f32   startSize     = 0.1f;
    f32   endSize       = 0.0f;
    AssetID materialID  = NULL_ASSET;
    bool  playing = true;
};

} // namespace Feliss
