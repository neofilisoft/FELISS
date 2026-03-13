#pragma once
#include "feliss/Types.h"
#include <string>
#include <unordered_map>
#include <functional>

namespace Feliss {

// =====================================================================
// Renderer-internal POD types — kept separate from Component.h
// so there is no cyclic dependency between renderer and ECS.
// =====================================================================

// ---- Draw commands ----
struct MeshDrawCmd {
    AssetID  meshID     = NULL_ASSET;
    AssetID  materialID = NULL_ASSET;
    Mat4     transform;
    u32      layer      = 1;
    bool     castShadow = true;
};

struct SpriteDrawCmd {
    AssetID   texID     = NULL_ASSET;
    Color     tint      = Color::white();
    Vec2      size      = {1,1};
    Vec3      worldPos  = Vec3::zero();
    f32       rot       = 0.0f;
    int       sortOrder = 0;
};

struct LightCmd {
    int   lightType  = 0;   // maps to LightComponent::LightType
    Color color      = Color::white();
    f32   intensity  = 1.0f;
    Vec3  position   = Vec3::zero();
    Vec3  direction  = {0,-1,0};
    f32   range      = 10.0f;
    f32   spotInner  = 15.0f;
    f32   spotOuter  = 30.0f;
    bool  castShadow = true;
};

// ---- Render pass ----
enum class RenderPassType : u32 {
    ShadowMap   = 0,
    Depth,
    Opaque,
    Transparent,
    Skybox,
    PostProcess,
    UI,
    Custom,
};

struct RenderPass {
    std::string    name;
    RenderPassType type        = RenderPassType::Custom;
    AssetID        targetID    = NULL_ASSET; // 0 = back buffer
    bool           enabled     = true;
    bool           clearColor  = true;
    bool           clearDepth  = true;
    Color          clearValue  = Color::black();
    u32            layerMask   = 0xFFFFFFFF;
    std::function<void()> customFn; // optional custom render
};

// ---- Post-process ----
struct PostEffect {
    std::string  name;
    AssetID      shaderID = NULL_ASSET;
    bool         enabled  = true;
    std::unordered_map<std::string, f32>   floats;
    std::unordered_map<std::string, Vec4>  vecs;
    std::unordered_map<std::string, Color> colors;
};

// ---- Render stats ----
struct RenderStats {
    u32 drawCalls     = 0;
    u32 spriteCalls   = 0;
    u32 triangles     = 0;
    u32 lightCount    = 0;
    u32 textureBinds  = 0;
    f32 gpuMs         = 0.0f;
    f32 cpuMs         = 0.0f;
};

// ---- Pipeline mode (affects overall render style) ----
enum class PipelineMode : u32 {
    PBR_3D   = 0,   // Full 3D PBR (Physically Based Rendering)
    HD2D     = 1,   // 2.5D: sprites in 3D world (Octopath style)
    Pixel2D  = 2,   // Classic pixel-art 2D
    Toon     = 3,   // Cel / toon shading
    Custom   = 4,   // Fully user-defined
};

inline const char* PipelineModeToString(PipelineMode m) {
    switch(m) {
        case PipelineMode::PBR_3D:  return "PBR 3D";
        case PipelineMode::HD2D:    return "HD-2D / 2.5D";
        case PipelineMode::Pixel2D: return "Pixel Art 2D";
        case PipelineMode::Toon:    return "Toon / Cel";
        case PipelineMode::Custom:  return "Custom";
        default:                    return "Unknown";
    }
}

// ---- Shader stage ----
enum class ShaderStage : u32 {
    Vertex=0, Fragment, Geometry, Compute, TessCtrl, TessEval,
};

// ---- Buffer usage ----
enum class BufferUsage : u32 {
    Static=0, Dynamic, Stream,
};

// ---- GPU handle (backend-specific integer) ----
using GpuHandle = u64;
static constexpr GpuHandle NULL_GPU_HANDLE = 0;

} // namespace Feliss
