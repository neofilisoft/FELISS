#include "renderer/RenderPipeline.h"
#include "core/Logger.h"
#include <algorithm>

namespace Feliss {

RenderPipeline::RenderPipeline(const RenderPipelineDesc& desc)
    : m_desc(desc), m_width(desc.width), m_height(desc.height)
{
    m_backend = createRendererBackend(desc.api);
}

RenderPipeline::~RenderPipeline() {
    shutdown();
}

bool RenderPipeline::init(Window& window) {
    if (!m_backend) {
        FLS_ERROR("RenderPipeline", "No backend created");
        return false;
    }
    if (!m_backend->init(window)) {
        FLS_ERROR("RenderPipeline", "Backend init failed");
        return false;
    }
    m_backend->setVSync(m_desc.vsync);
    buildDefaultPasses();
    FLS_INFOF("RenderPipeline", "RenderPipeline initialized ["
        << m_backend->apiName() << "] "
        << m_width << "x" << m_height);
    return true;
}

void RenderPipeline::shutdown() {
    if (m_backend) { m_backend->shutdown(); }
}

void RenderPipeline::onResize(int w, int h) {
    m_width = w; m_height = h;
    if (m_backend) m_backend->onResize(w, h);
}

void RenderPipeline::beginFrame() {
    if (!m_backend || m_inFrame) return;
    m_inFrame = true;
    m_stats   = {};
    m_meshQueue.clear();
    m_spriteQueue.clear();
    m_lights.clear();
    m_backend->beginFrame();
}

void RenderPipeline::endFrame() {
    if (!m_backend || !m_inFrame) return;
    flushMeshQueue();
    flushSpriteQueue();
    runPostProcess();
    m_backend->endFrame();
    m_backend->present();
    m_inFrame = false;
}

void RenderPipeline::beginImGui() {
    if (m_backend) m_backend->imguiNewFrame();
}

void RenderPipeline::endImGui() {
    if (m_backend) m_backend->imguiRender();
}

// ---- Submit ----
void RenderPipeline::submitMesh(AssetID mesh, AssetID mat,
                                 const Mat4& transform,
                                 bool castShadow, u32 layer) {
    MeshDrawCmd cmd;
    cmd.meshID     = mesh;
    cmd.materialID = mat;
    cmd.transform  = transform;
    cmd.castShadow = castShadow;
    cmd.layer      = layer;
    m_meshQueue.push_back(cmd);
    m_stats.drawCalls++;
}

void RenderPipeline::submitSprite(AssetID tex, const Color& tint,
                                   const Vec2& size, const Vec3& worldPos,
                                   int sortOrder) {
    SpriteDrawCmd cmd;
    cmd.texID     = tex;
    cmd.tint      = tint;
    cmd.size      = size;
    cmd.worldPos  = worldPos;
    cmd.sortOrder = sortOrder;
    m_spriteQueue.push_back(cmd);
    m_stats.spriteCalls++;
}

void RenderPipeline::submitLight(int lightType, const Color& color,
                                  f32 intensity, const Vec3& pos,
                                  const Vec3& dir, f32 range) {
    LightCmd l;
    l.lightType = lightType;
    l.color     = color;
    l.intensity = intensity;
    l.position  = pos;
    l.direction = dir;
    l.range     = range;
    m_lights.push_back(l);
    m_stats.lightCount++;
}

// ---- Passes ----
void RenderPipeline::buildDefaultPasses() {
    m_passes.clear();

    RenderPass shadow;
    shadow.name    = "ShadowMap";
    shadow.type    = RenderPassType::ShadowMap;
    shadow.enabled = true;
    m_passes.push_back(shadow);

    RenderPass opaque;
    opaque.name    = "Opaque";
    opaque.type    = RenderPassType::Opaque;
    opaque.enabled = true;
    m_passes.push_back(opaque);

    RenderPass skybox;
    skybox.name    = "Skybox";
    skybox.type    = RenderPassType::Skybox;
    skybox.enabled = true;
    m_passes.push_back(skybox);

    RenderPass transparent;
    transparent.name    = "Transparent";
    transparent.type    = RenderPassType::Transparent;
    transparent.enabled = true;
    m_passes.push_back(transparent);

    RenderPass pp;
    pp.name    = "PostProcess";
    pp.type    = RenderPassType::PostProcess;
    pp.enabled = true;
    m_passes.push_back(pp);

    RenderPass ui;
    ui.name    = "UI";
    ui.type    = RenderPassType::UI;
    ui.enabled = true;
    m_passes.push_back(ui);
}

void RenderPipeline::addPass(const RenderPass& pass) {
    m_passes.push_back(pass);
}

void RenderPipeline::removePass(const std::string& name) {
    m_passes.erase(std::remove_if(m_passes.begin(), m_passes.end(),
        [&](const RenderPass& p){ return p.name == name; }), m_passes.end());
}

RenderPass* RenderPipeline::getPass(const std::string& name) {
    for (auto& p : m_passes) if (p.name == name) return &p;
    return nullptr;
}

void RenderPipeline::setPassEnabled(const std::string& name, bool en) {
    if (auto* p = getPass(name)) p->enabled = en;
}

void RenderPipeline::addPostEffect(const PostEffect& fx) {
    m_postEffects.push_back(fx);
}

void RenderPipeline::removePostEffect(const std::string& name) {
    m_postEffects.erase(std::remove_if(m_postEffects.begin(), m_postEffects.end(),
        [&](const PostEffect& e){ return e.name == name; }), m_postEffects.end());
}

void RenderPipeline::setPostEffectEnabled(const std::string& name, bool en) {
    for (auto& e : m_postEffects) if (e.name == name) { e.enabled = en; return; }
}

GpuHandle RenderPipeline::createRenderTarget(int w, int h, PixelFormat fmt, bool hasDepth) {
    if (!m_backend) return NULL_GPU_HANDLE;
    return m_backend->createFramebuffer(w, h, fmt, hasDepth);
}

void RenderPipeline::destroyRenderTarget(GpuHandle h) {
    if (m_backend) m_backend->destroyFramebuffer(h);
}

GpuHandle RenderPipeline::getRenderTargetColorTex(GpuHandle h) {
    if (!m_backend) return NULL_GPU_HANDLE;
    return m_backend->framebufferColorTexture(h);
}

void RenderPipeline::setPipelineMode(PipelineMode m) {
    m_mode = m;
    FLS_INFOF("RenderPipeline", "Pipeline mode: " << PipelineModeToString(m));
}

void RenderPipeline::setSkybox(AssetID id)   { m_skyboxID = id; }
void RenderPipeline::setAmbient(const Color& c) { m_ambient = c; }
void RenderPipeline::setFog(bool en, const Color& c, f32 density) {
    m_fogEnabled = en; m_fogColor = c; m_fogDensity = density;
}

void RenderPipeline::flushMeshQueue() {
    if (m_meshQueue.empty() || !m_backend) return;
    // Sort by layer, then by material for batching
    std::sort(m_meshQueue.begin(), m_meshQueue.end(),
        [](const MeshDrawCmd& a, const MeshDrawCmd& b){
            if (a.layer != b.layer) return a.layer < b.layer;
            return a.materialID < b.materialID;
        });
    // TODO: bind materials, set uniforms, draw each mesh
    m_stats.triangles += static_cast<u32>(m_meshQueue.size() * 100); // placeholder
}

void RenderPipeline::flushSpriteQueue() {
    if (m_spriteQueue.empty() || !m_backend) return;
    // Sort sprites by sort order (painter's algorithm)
    std::sort(m_spriteQueue.begin(), m_spriteQueue.end(),
        [](const SpriteDrawCmd& a, const SpriteDrawCmd& b){
            return a.sortOrder < b.sortOrder;
        });
    // TODO: batch-render sprites via sprite batcher
}

void RenderPipeline::runPostProcess() {
    if (!m_backend) return;
    for (auto& fx : m_postEffects) {
        if (!fx.enabled || fx.shaderID == NULL_ASSET) continue;
        // TODO: bind fullscreen quad shader and render
    }
}

} // namespace Feliss
