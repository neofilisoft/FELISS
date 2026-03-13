#pragma once
#include "renderer/IRendererBackend.h"
#include <unordered_map>
#include <string>

namespace Feliss {

// =====================================================================
// OpenGLBackend — OpenGL 4.5 Core Profile
// Requires: GLAD loader + GLFW window
// =====================================================================
class OpenGLBackend : public IRendererBackend {
public:
    OpenGLBackend() = default;
    ~OpenGLBackend() override;

    bool        init(Window& window)     override;
    void        shutdown()               override;
    const char* apiName() const override { return "OpenGL 4.5"; }
    RenderAPI   api()     const override { return RenderAPI::OpenGL; }

    void beginFrame()                    override;
    void endFrame()                      override;
    void present()                       override;
    void setVSync(bool on)               override;
    void onResize(int w, int h)          override;

    void setViewport(const Viewport& v)  override;
    void setScissor(int x,int y,int w,int h) override;
    void setClearColor(const Color& c)   override;
    void clear(bool col,bool dep,bool sten) override;
    void setBlendMode(BlendMode m)       override;
    void setCullMode(CullMode m)         override;
    void setFillMode(FillMode m)         override;
    void setDepthTest(bool e)            override;
    void setDepthWrite(bool e)           override;

    GpuHandle createShader(ShaderStage stage, const char* src, bool isGLSL) override;
    GpuHandle createProgram(GpuHandle vert, GpuHandle frag, GpuHandle geom)  override;
    void      bindProgram(GpuHandle prog)  override;
    void      destroyShader(GpuHandle sh)  override;
    void      destroyProgram(GpuHandle p)  override;

    void setUniformI(GpuHandle p, const char* n, int v)           override;
    void setUniformF(GpuHandle p, const char* n, f32 v)           override;
    void setUniformV2(GpuHandle p, const char* n, Vec2 v)         override;
    void setUniformV3(GpuHandle p, const char* n, Vec3 v)         override;
    void setUniformV4(GpuHandle p, const char* n, Vec4 v)         override;
    void setUniformM4(GpuHandle p, const char* n, const Mat4& m)  override;

    GpuHandle createVertexBuffer(const void* d, size_t b, BufferUsage u) override;
    GpuHandle createIndexBuffer(const void* d, size_t b, BufferUsage u)  override;
    void      updateBuffer(GpuHandle buf, const void* d, size_t b)        override;
    void      bindVertexBuffer(GpuHandle buf)   override;
    void      bindIndexBuffer(GpuHandle buf)    override;
    void      destroyBuffer(GpuHandle buf)      override;

    GpuHandle createVertexArray()                                   override;
    void      bindVertexArray(GpuHandle vao)                        override;
    void      setVertexAttrib(u32 idx, int size, bool norm, int stride, size_t off) override;
    void      destroyVertexArray(GpuHandle vao)                     override;

    GpuHandle createTexture2D(int w, int h, PixelFormat fmt, const void* data,
                              TextureFilter filter, TextureWrap wrap)   override;
    GpuHandle createCubemap(int sz, PixelFormat fmt, const void* faces[6]) override;
    void      bindTexture(GpuHandle tex, u32 slot)                      override;
    void      updateTexture2D(GpuHandle tex, int x, int y, int w, int h, const void* d) override;
    void      destroyTexture(GpuHandle tex)                             override;

    GpuHandle createFramebuffer(int w, int h, PixelFormat fmt, bool dep) override;
    void      bindFramebuffer(GpuHandle fb)                             override;
    GpuHandle framebufferColorTexture(GpuHandle fb)                     override;
    GpuHandle framebufferDepthTexture(GpuHandle fb)                     override;
    void      blitFramebuffer(GpuHandle src, GpuHandle dst,
                              int sw, int sh, int dw, int dh)           override;
    void      destroyFramebuffer(GpuHandle fb)                          override;

    void drawArrays(u32 mode, int first, int count)                     override;
    void drawIndexed(u32 mode, int count, bool u32Idx)                  override;
    void drawArraysInstanced(u32 mode, int first, int count, int inst)  override;
    void drawIndexedInstanced(u32 mode, int cnt, int inst, bool u32Idx) override;

    bool      supportsCompute() const override { return true; }
    GpuHandle createComputeShader(const char* src) override;
    void      dispatchCompute(GpuHandle cs, u32 x, u32 y, u32 z) override;

    void imguiInit()     override;
    void imguiNewFrame() override;
    void imguiRender()   override;
    void imguiShutdown() override;

    std::string deviceName()    const override;
    std::string driverVersion() const override;
    int         maxTextureSize()const override;

private:
    u32 toGLUsage(BufferUsage u) const;
    u32 toGLFormat(PixelFormat f, bool* isDepth = nullptr) const;
    u32 toGLInternal(PixelFormat f) const;
    u32 toGLType(PixelFormat f) const;
    u32 toGLFilter(TextureFilter f) const;
    u32 toGLWrap(TextureWrap w) const;
    int uniformLocation(GpuHandle prog, const char* name);

    // Framebuffer helper data
    struct FBData { u32 fbo=0, colorTex=0, depthTex=0; };
    std::unordered_map<GpuHandle, FBData>     m_fbData;
    std::unordered_map<GpuHandle,
        std::unordered_map<std::string,int>>  m_uniformCache;

    void*   m_glfwWindow = nullptr;  // GLFWwindow*
    bool    m_initialized = false;
    Color   m_clearColor  = Color::black();
};

} // namespace Feliss
