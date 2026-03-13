#pragma once
#include "RenderTypes.h"
#include <string>

namespace Feliss {

class Window;

// =====================================================================
// IRendererBackend — Pure interface implemented per graphics API.
//
//  Concrete backends:
//    OpenGLBackend    (src/renderer/backends/OpenGLBackend.h)
//    VulkanBackend    (src/renderer/backends/VulkanBackend.h)
//    D3D11Backend     (src/renderer/backends/D3D11Backend.h)
//    D3D12Backend     (src/renderer/backends/D3D12Backend.h)
//    MetalBackend     (src/renderer/backends/MetalBackend.h)
//    WebGLBackend     (future)
//
// The RenderPipeline owns one backend selected at runtime.
// =====================================================================
class IRendererBackend {
public:
    virtual ~IRendererBackend() = default;

    // ---- Lifecycle ----
    virtual bool        init(Window& window)          = 0;
    virtual void        shutdown()                    = 0;
    virtual const char* apiName()              const  = 0;
    virtual RenderAPI   api()                  const  = 0;

    // ---- Frame ----
    virtual void beginFrame()                         = 0;
    virtual void endFrame()                           = 0;
    virtual void present()                            = 0;
    virtual void setVSync(bool enabled)               = 0;
    virtual void onResize(int w, int h)               = 0;

    // ---- State ----
    virtual void setViewport(const Viewport& vp)      = 0;
    virtual void setScissor(int x,int y,int w,int h)  = 0;
    virtual void setClearColor(const Color& c)        = 0;
    virtual void clear(bool color, bool depth, bool stencil) = 0;
    virtual void setBlendMode(BlendMode mode)         = 0;
    virtual void setCullMode(CullMode mode)           = 0;
    virtual void setFillMode(FillMode mode)           = 0;
    virtual void setDepthTest(bool enable)            = 0;
    virtual void setDepthWrite(bool enable)           = 0;

    // ---- Shaders ----
    virtual GpuHandle createShader(ShaderStage stage,
                                   const char* src,
                                   bool isGLSL = true)       = 0;
    virtual GpuHandle createProgram(GpuHandle vert,
                                    GpuHandle frag,
                                    GpuHandle geom = NULL_GPU_HANDLE) = 0;
    virtual void      bindProgram(GpuHandle prog)            = 0;
    virtual void      destroyShader(GpuHandle sh)            = 0;
    virtual void      destroyProgram(GpuHandle prog)         = 0;

    // Uniform setters
    virtual void setUniformI(GpuHandle prog, const char* name, int v)     = 0;
    virtual void setUniformF(GpuHandle prog, const char* name, f32 v)     = 0;
    virtual void setUniformV2(GpuHandle prog, const char* name, Vec2 v)   = 0;
    virtual void setUniformV3(GpuHandle prog, const char* name, Vec3 v)   = 0;
    virtual void setUniformV4(GpuHandle prog, const char* name, Vec4 v)   = 0;
    virtual void setUniformM4(GpuHandle prog, const char* name, const Mat4& m) = 0;

    // ---- Buffers ----
    virtual GpuHandle createVertexBuffer(const void* data, size_t bytes,
                                         BufferUsage usage = BufferUsage::Static) = 0;
    virtual GpuHandle createIndexBuffer(const void* data, size_t bytes,
                                        BufferUsage usage = BufferUsage::Static)  = 0;
    virtual void      updateBuffer(GpuHandle buf, const void* data, size_t bytes)  = 0;
    virtual void      bindVertexBuffer(GpuHandle buf)                               = 0;
    virtual void      bindIndexBuffer(GpuHandle buf)                                = 0;
    virtual void      destroyBuffer(GpuHandle buf)                                  = 0;

    // ---- Vertex layout ----
    virtual GpuHandle createVertexArray()                          = 0;
    virtual void      bindVertexArray(GpuHandle vao)               = 0;
    virtual void      setVertexAttrib(u32 idx, int size, bool normalized,
                                      int stride, size_t offset)   = 0;
    virtual void      destroyVertexArray(GpuHandle vao)            = 0;

    // ---- Textures ----
    virtual GpuHandle createTexture2D(int w, int h, PixelFormat fmt,
                                      const void* data,
                                      TextureFilter filter = TextureFilter::Linear,
                                      TextureWrap wrap     = TextureWrap::Repeat) = 0;
    virtual GpuHandle createCubemap(int size, PixelFormat fmt,
                                    const void* faces[6])                          = 0;
    virtual void      bindTexture(GpuHandle tex, u32 slot)                         = 0;
    virtual void      updateTexture2D(GpuHandle tex, int x, int y,
                                      int w, int h, const void* data)              = 0;
    virtual void      destroyTexture(GpuHandle tex)                                = 0;

    // ---- Render targets / Framebuffers ----
    virtual GpuHandle createFramebuffer(int w, int h, PixelFormat colorFmt,
                                        bool hasDepth)                 = 0;
    virtual void      bindFramebuffer(GpuHandle fb)                    = 0;  // 0 = default
    virtual GpuHandle framebufferColorTexture(GpuHandle fb)            = 0;
    virtual GpuHandle framebufferDepthTexture(GpuHandle fb)            = 0;
    virtual void      blitFramebuffer(GpuHandle src, GpuHandle dst,
                                      int srcW, int srcH,
                                      int dstW, int dstH)              = 0;
    virtual void      destroyFramebuffer(GpuHandle fb)                 = 0;

    // ---- Draw ----
    virtual void drawArrays(u32 mode, int first, int count)                        = 0;
    virtual void drawIndexed(u32 mode, int count, bool u32Indices = true)          = 0;
    virtual void drawArraysInstanced(u32 mode, int first,
                                     int count, int instances)                     = 0;
    virtual void drawIndexedInstanced(u32 mode, int count,
                                      int instances, bool u32Idx = true)           = 0;

    // ---- Compute (Vulkan / D3D12 / Metal) ----
    virtual bool        supportsCompute() const { return false; }
    virtual GpuHandle   createComputeShader(const char* src) { return NULL_GPU_HANDLE; }
    virtual void        dispatchCompute(GpuHandle cs,u32 x,u32 y,u32 z) {}

    // ---- ImGui integration ----
    virtual void imguiInit()                                      = 0;
    virtual void imguiNewFrame()                                  = 0;
    virtual void imguiRender()                                    = 0;
    virtual void imguiShutdown()                                  = 0;

    // ---- Device info ----
    virtual std::string deviceName()   const { return "Unknown"; }
    virtual std::string driverVersion()const { return ""; }
    virtual int         maxTextureSize()const { return 4096; }
    virtual bool        supportsFeature(const char* feat) const { return false; }
};

// ---- Factory ----
Scope<IRendererBackend> createRendererBackend(RenderAPI api);

} // namespace Feliss
