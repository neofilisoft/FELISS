#pragma once
// =====================================================================
// MetalBackend — Apple Metal stub (macOS / iOS only)
// Full implementation requires: Apple Metal framework, MetalKit
// =====================================================================
#include "renderer/IRendererBackend.h"

namespace Feliss {

class MetalBackend : public IRendererBackend {
public:
    MetalBackend()  = default;
    ~MetalBackend() = default;

    bool        init(Window& w)      override;
    void        shutdown()           override;
    const char* apiName()  const override { return "Metal 3"; }
    RenderAPI   api()      const override { return RenderAPI::Metal; }

    void beginFrame()      override {}
    void endFrame()        override {}
    void present()         override {}
    void setVSync(bool)    override {}
    void onResize(int,int) override {}
    void setViewport(const Viewport&) override {}
    void setScissor(int,int,int,int)  override {}
    void setClearColor(const Color&)  override {}
    void clear(bool,bool,bool)        override {}
    void setBlendMode(BlendMode)      override {}
    void setCullMode(CullMode)        override {}
    void setFillMode(FillMode)        override {}
    void setDepthTest(bool)           override {}
    void setDepthWrite(bool)          override {}

    GpuHandle createShader(ShaderStage,const char*,bool) override { return NULL_GPU_HANDLE; }
    GpuHandle createProgram(GpuHandle,GpuHandle,GpuHandle) override { return NULL_GPU_HANDLE; }
    void      bindProgram(GpuHandle)    override {}
    void      destroyShader(GpuHandle)  override {}
    void      destroyProgram(GpuHandle) override {}

    void setUniformI(GpuHandle,const char*,int)          override {}
    void setUniformF(GpuHandle,const char*,f32)          override {}
    void setUniformV2(GpuHandle,const char*,Vec2)        override {}
    void setUniformV3(GpuHandle,const char*,Vec3)        override {}
    void setUniformV4(GpuHandle,const char*,Vec4)        override {}
    void setUniformM4(GpuHandle,const char*,const Mat4&) override {}

    GpuHandle createVertexBuffer(const void*,size_t,BufferUsage) override { return NULL_GPU_HANDLE; }
    GpuHandle createIndexBuffer(const void*,size_t,BufferUsage)  override { return NULL_GPU_HANDLE; }
    void      updateBuffer(GpuHandle,const void*,size_t) override {}
    void      bindVertexBuffer(GpuHandle)   override {}
    void      bindIndexBuffer(GpuHandle)    override {}
    void      destroyBuffer(GpuHandle)      override {}

    GpuHandle createVertexArray()           override { return NULL_GPU_HANDLE; }
    void      bindVertexArray(GpuHandle)    override {}
    void      setVertexAttrib(u32,int,bool,int,size_t) override {}
    void      destroyVertexArray(GpuHandle) override {}

    GpuHandle createTexture2D(int,int,PixelFormat,const void*,TextureFilter,TextureWrap) override { return NULL_GPU_HANDLE; }
    GpuHandle createCubemap(int,PixelFormat,const void*[6]) override { return NULL_GPU_HANDLE; }
    void      bindTexture(GpuHandle,u32)    override {}
    void      updateTexture2D(GpuHandle,int,int,int,int,const void*) override {}
    void      destroyTexture(GpuHandle)     override {}

    GpuHandle createFramebuffer(int,int,PixelFormat,bool) override { return NULL_GPU_HANDLE; }
    void      bindFramebuffer(GpuHandle)    override {}
    GpuHandle framebufferColorTexture(GpuHandle) override { return NULL_GPU_HANDLE; }
    GpuHandle framebufferDepthTexture(GpuHandle) override { return NULL_GPU_HANDLE; }
    void      blitFramebuffer(GpuHandle,GpuHandle,int,int,int,int) override {}
    void      destroyFramebuffer(GpuHandle) override {}

    void drawArrays(u32,int,int)                override {}
    void drawIndexed(u32,int,bool)              override {}
    void drawArraysInstanced(u32,int,int,int)   override {}
    void drawIndexedInstanced(u32,int,int,bool) override {}

    bool supportsCompute() const override { return true; }

    void imguiInit()     override {}
    void imguiNewFrame() override {}
    void imguiRender()   override {}
    void imguiShutdown() override {}
};

} // namespace Feliss
