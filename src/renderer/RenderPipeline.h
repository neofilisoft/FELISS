#pragma once
#include "renderer/RenderTypes.h"
#include "renderer/IRendererBackend.h"
#include <vector>
#include <string>
#include <memory>

namespace Feliss {

class Window;

// =====================================================================
// RenderPipeline — owns the backend, manages passes & draw queue
// =====================================================================
struct RenderPipelineDesc {
    RenderAPI api    = RenderAPI::OpenGL;
    int       width  = 1280;
    int       height = 720;
    bool      vsync  = true;
    int       msaa   = 4;
};

class RenderPipeline {
public:
    explicit RenderPipeline(const RenderPipelineDesc& desc);
    ~RenderPipeline();
    RenderPipeline(const RenderPipeline&)            = delete;
    RenderPipeline& operator=(const RenderPipeline&) = delete;

    // Lifecycle
    bool init(Window& window);
    void shutdown();
    void onResize(int w, int h);

    // Frame
    void beginFrame();
    void endFrame();
    void beginImGui();
    void endImGui();

    // Submit draw commands (called by World::render)
    void submitMesh(AssetID mesh, AssetID mat,
                    const Mat4& transform,
                    bool castShadow = true, u32 layer = 1);
    void submitSprite(AssetID tex, const Color& tint,
                      const Vec2& size, const Vec3& worldPos,
                      int sortOrder = 0);
    void submitLight(int lightType, const Color& color,
                     f32 intensity, const Vec3& pos,
                     const Vec3& dir, f32 range);

    // Pass management
    void addPass(const RenderPass& pass);
    void removePass(const std::string& name);
    RenderPass* getPass(const std::string& name);
    const std::vector<RenderPass>& passes() const { return m_passes; }
    void setPassEnabled(const std::string& name, bool en);

    // Post-processing
    void addPostEffect(const PostEffect& fx);
    void removePostEffect(const std::string& name);
    void setPostEffectEnabled(const std::string& name, bool en);
    const std::vector<PostEffect>& postEffects() const { return m_postEffects; }

    // Render targets
    GpuHandle createRenderTarget(int w, int h,
                                  PixelFormat fmt = PixelFormat::RGBA8,
                                  bool hasDepth = true);
    void destroyRenderTarget(GpuHandle h);
    GpuHandle getRenderTargetColorTex(GpuHandle h);

    // Pipeline mode
    void         setPipelineMode(PipelineMode m);
    PipelineMode getPipelineMode() const { return m_mode; }

    // Environment
    void setSkybox(AssetID id);
    void setAmbient(const Color& c);
    void setFog(bool en, const Color& c = Color::white(), f32 density = 0.01f);

    // Stats & accessors
    const RenderStats&  stats()    const { return m_stats; }
    RenderAPI           api()      const { return m_desc.api; }
    int                 width()    const { return m_width; }
    int                 height()   const { return m_height; }
    IRendererBackend&   backend()        { return *m_backend; }

private:
    void buildDefaultPasses();
    void flushMeshQueue();
    void flushSpriteQueue();
    void runPostProcess();

    RenderPipelineDesc m_desc;
    int                m_width  = 1280;
    int                m_height = 720;
    PipelineMode       m_mode   = PipelineMode::PBR_3D;
    bool               m_inFrame = false;

    Scope<IRendererBackend> m_backend;
    std::vector<RenderPass> m_passes;
    std::vector<PostEffect> m_postEffects;
    std::vector<MeshDrawCmd>   m_meshQueue;
    std::vector<SpriteDrawCmd> m_spriteQueue;
    std::vector<LightCmd>      m_lights;

    AssetID m_skyboxID    = NULL_ASSET;
    Color   m_ambient     = {0.1f,0.1f,0.1f,1.0f};
    bool    m_fogEnabled  = false;
    Color   m_fogColor    = Color::white();
    f32     m_fogDensity  = 0.01f;

    RenderStats m_stats;
};

} // namespace Feliss
