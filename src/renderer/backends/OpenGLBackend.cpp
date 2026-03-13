#include "renderer/backends/OpenGLBackend.h"
#include "core/Logger.h"

// Guard entire file behind compile flag
#ifdef FELISS_RENDERER_OPENGL

// GLAD must be included BEFORE GLFW
#ifdef FELISS_HAS_GLAD
#  include <glad/glad.h>
#endif

#ifdef FELISS_HAS_GLFW
#  include <GLFW/glfw3.h>
#endif

#ifdef FELISS_EDITOR_BUILD
#  ifdef FELISS_HAS_IMGUI
#    include <imgui.h>
#    include <backends/imgui_impl_glfw.h>
#    include <backends/imgui_impl_opengl3.h>
#  endif
#endif

#include <cstring>

namespace Feliss {

OpenGLBackend::~OpenGLBackend() {
    if (m_initialized) shutdown();
}

bool OpenGLBackend::init(Window& window) {
#if defined(FELISS_HAS_GLAD) && defined(FELISS_HAS_GLFW)
    // GLAD: load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        FLS_ERROR("OpenGL", "Failed to initialize GLAD");
        return false;
    }
    FLS_INFOF("OpenGL", "Renderer: " << (const char*)glGetString(GL_RENDERER));
    FLS_INFOF("OpenGL", "Version: "  << (const char*)glGetString(GL_VERSION));

    // Default state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_initialized = true;
    FLS_INFO("OpenGL", "OpenGL backend initialized");
    return true;
#else
    FLS_ERROR("OpenGL", "GLAD or GLFW not compiled in — cannot initialize OpenGL backend");
    return false;
#endif
}

void OpenGLBackend::shutdown() {
#if defined(FELISS_HAS_GLAD)
    m_initialized = false;
    FLS_INFO("OpenGL", "OpenGL backend shutdown");
#endif
}

void OpenGLBackend::beginFrame() {
#ifdef FELISS_HAS_GLAD
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}

void OpenGLBackend::endFrame() {
    // Framebuffer swap handled by Window
}

void OpenGLBackend::present() {
#ifdef FELISS_HAS_GLFW
    if (m_glfwWindow)
        glfwSwapBuffers(static_cast<GLFWwindow*>(m_glfwWindow));
#endif
}

void OpenGLBackend::setVSync(bool on) {
#ifdef FELISS_HAS_GLFW
    glfwSwapInterval(on ? 1 : 0);
#endif
}

void OpenGLBackend::onResize(int w, int h) {
#ifdef FELISS_HAS_GLAD
    glViewport(0, 0, w, h);
#endif
}

void OpenGLBackend::setViewport(const Viewport& v) {
#ifdef FELISS_HAS_GLAD
    glViewport((GLint)v.x, (GLint)v.y, (GLsizei)v.width, (GLsizei)v.height);
    glDepthRangef(v.minDepth, v.maxDepth);
#endif
}

void OpenGLBackend::setScissor(int x, int y, int w, int h) {
#ifdef FELISS_HAS_GLAD
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w, h);
#endif
}

void OpenGLBackend::setClearColor(const Color& c) {
    m_clearColor = c;
}

void OpenGLBackend::clear(bool col, bool dep, bool sten) {
#ifdef FELISS_HAS_GLAD
    GLbitfield mask = 0;
    if (col)  mask |= GL_COLOR_BUFFER_BIT;
    if (dep)  mask |= GL_DEPTH_BUFFER_BIT;
    if (sten) mask |= GL_STENCIL_BUFFER_BIT;
    if (mask) {
        glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
        glClear(mask);
    }
#endif
}

void OpenGLBackend::setBlendMode(BlendMode m) {
#ifdef FELISS_HAS_GLAD
    switch(m) {
        case BlendMode::Opaque:
            glDisable(GL_BLEND); break;
        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE); break;
        case BlendMode::Multiply:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO); break;
        case BlendMode::Screen:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR); break;
    }
#endif
}

void OpenGLBackend::setCullMode(CullMode m) {
#ifdef FELISS_HAS_GLAD
    if (m == CullMode::None) { glDisable(GL_CULL_FACE); return; }
    glEnable(GL_CULL_FACE);
    glCullFace(m == CullMode::Front ? GL_FRONT : GL_BACK);
#endif
}

void OpenGLBackend::setFillMode(FillMode m) {
#ifdef FELISS_HAS_GLAD
    glPolygonMode(GL_FRONT_AND_BACK, m == FillMode::Wireframe ? GL_LINE : GL_FILL);
#endif
}

void OpenGLBackend::setDepthTest(bool e) {
#ifdef FELISS_HAS_GLAD
    e ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
#endif
}

void OpenGLBackend::setDepthWrite(bool e) {
#ifdef FELISS_HAS_GLAD
    glDepthMask(e ? GL_TRUE : GL_FALSE);
#endif
}

// ---- Shaders ----
GpuHandle OpenGLBackend::createShader(ShaderStage stage, const char* src, bool /*isGLSL*/) {
#ifdef FELISS_HAS_GLAD
    GLenum type = GL_VERTEX_SHADER;
    switch(stage) {
        case ShaderStage::Fragment: type = GL_FRAGMENT_SHADER;  break;
        case ShaderStage::Geometry: type = GL_GEOMETRY_SHADER;  break;
        case ShaderStage::Compute:  type = GL_COMPUTE_SHADER;   break;
        case ShaderStage::TessCtrl: type = GL_TESS_CONTROL_SHADER; break;
        case ShaderStage::TessEval: type = GL_TESS_EVALUATION_SHADER; break;
        default: break;
    }
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetShaderInfoLog(sh, sizeof(buf), nullptr, buf);
        FLS_ERRORF("OpenGL", "Shader compile error: " << buf);
        glDeleteShader(sh);
        return NULL_GPU_HANDLE;
    }
    return static_cast<GpuHandle>(sh);
#else
    return NULL_GPU_HANDLE;
#endif
}

GpuHandle OpenGLBackend::createProgram(GpuHandle vert, GpuHandle frag, GpuHandle geom) {
#ifdef FELISS_HAS_GLAD
    GLuint prog = glCreateProgram();
    if (vert) glAttachShader(prog, static_cast<GLuint>(vert));
    if (frag) glAttachShader(prog, static_cast<GLuint>(frag));
    if (geom) glAttachShader(prog, static_cast<GLuint>(geom));
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        FLS_ERRORF("OpenGL", "Program link error: " << buf);
        glDeleteProgram(prog);
        return NULL_GPU_HANDLE;
    }
    return static_cast<GpuHandle>(prog);
#else
    return NULL_GPU_HANDLE;
#endif
}

void OpenGLBackend::bindProgram(GpuHandle prog) {
#ifdef FELISS_HAS_GLAD
    glUseProgram(static_cast<GLuint>(prog));
#endif
}

void OpenGLBackend::destroyShader(GpuHandle sh) {
#ifdef FELISS_HAS_GLAD
    if (sh) glDeleteShader(static_cast<GLuint>(sh));
#endif
}

void OpenGLBackend::destroyProgram(GpuHandle p) {
#ifdef FELISS_HAS_GLAD
    if (p) { m_uniformCache.erase(p); glDeleteProgram(static_cast<GLuint>(p)); }
#endif
}

int OpenGLBackend::uniformLocation(GpuHandle prog, const char* name) {
#ifdef FELISS_HAS_GLAD
    auto& cache = m_uniformCache[prog];
    auto it = cache.find(name);
    if (it != cache.end()) return it->second;
    int loc = glGetUniformLocation(static_cast<GLuint>(prog), name);
    cache[name] = loc;
    return loc;
#else
    return -1;
#endif
}

void OpenGLBackend::setUniformI(GpuHandle p, const char* n, int v) {
#ifdef FELISS_HAS_GLAD
    glUniform1i(uniformLocation(p,n), v);
#endif
}
void OpenGLBackend::setUniformF(GpuHandle p, const char* n, f32 v) {
#ifdef FELISS_HAS_GLAD
    glUniform1f(uniformLocation(p,n), v);
#endif
}
void OpenGLBackend::setUniformV2(GpuHandle p, const char* n, Vec2 v) {
#ifdef FELISS_HAS_GLAD
    glUniform2f(uniformLocation(p,n), v.x, v.y);
#endif
}
void OpenGLBackend::setUniformV3(GpuHandle p, const char* n, Vec3 v) {
#ifdef FELISS_HAS_GLAD
    glUniform3f(uniformLocation(p,n), v.x, v.y, v.z);
#endif
}
void OpenGLBackend::setUniformV4(GpuHandle p, const char* n, Vec4 v) {
#ifdef FELISS_HAS_GLAD
    glUniform4f(uniformLocation(p,n), v.x, v.y, v.z, v.w);
#endif
}
void OpenGLBackend::setUniformM4(GpuHandle p, const char* n, const Mat4& m) {
#ifdef FELISS_HAS_GLAD
    glUniformMatrix4fv(uniformLocation(p,n), 1, GL_FALSE, m.data());
#endif
}

// ---- Buffers ----
u32 OpenGLBackend::toGLUsage(BufferUsage u) const {
#ifdef FELISS_HAS_GLAD
    switch(u) {
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        default:                   return GL_STATIC_DRAW;
    }
#else
    return 0;
#endif
}

GpuHandle OpenGLBackend::createVertexBuffer(const void* d, size_t b, BufferUsage u) {
#ifdef FELISS_HAS_GLAD
    GLuint buf = 0;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)b, d, toGLUsage(u));
    return static_cast<GpuHandle>(buf);
#else
    return NULL_GPU_HANDLE;
#endif
}

GpuHandle OpenGLBackend::createIndexBuffer(const void* d, size_t b, BufferUsage u) {
#ifdef FELISS_HAS_GLAD
    GLuint buf = 0;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)b, d, toGLUsage(u));
    return static_cast<GpuHandle>(buf);
#else
    return NULL_GPU_HANDLE;
#endif
}

void OpenGLBackend::updateBuffer(GpuHandle buf, const void* d, size_t b) {
#ifdef FELISS_HAS_GLAD
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(buf));
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)b, d);
#endif
}

void OpenGLBackend::bindVertexBuffer(GpuHandle buf) {
#ifdef FELISS_HAS_GLAD
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(buf));
#endif
}

void OpenGLBackend::bindIndexBuffer(GpuHandle buf) {
#ifdef FELISS_HAS_GLAD
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(buf));
#endif
}

void OpenGLBackend::destroyBuffer(GpuHandle buf) {
#ifdef FELISS_HAS_GLAD
    GLuint b = static_cast<GLuint>(buf);
    if (b) glDeleteBuffers(1, &b);
#endif
}

// ---- Vertex Array ----
GpuHandle OpenGLBackend::createVertexArray() {
#ifdef FELISS_HAS_GLAD
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    return static_cast<GpuHandle>(vao);
#else
    return NULL_GPU_HANDLE;
#endif
}
void OpenGLBackend::bindVertexArray(GpuHandle vao) {
#ifdef FELISS_HAS_GLAD
    glBindVertexArray(static_cast<GLuint>(vao));
#endif
}
void OpenGLBackend::setVertexAttrib(u32 idx, int size, bool norm, int stride, size_t off) {
#ifdef FELISS_HAS_GLAD
    glEnableVertexAttribArray(idx);
    glVertexAttribPointer(idx, size, GL_FLOAT,
        norm ? GL_TRUE : GL_FALSE,
        stride, reinterpret_cast<const void*>(off));
#endif
}
void OpenGLBackend::destroyVertexArray(GpuHandle vao) {
#ifdef FELISS_HAS_GLAD
    GLuint v = static_cast<GLuint>(vao);
    if (v) glDeleteVertexArrays(1, &v);
#endif
}

// ---- Textures ----
u32 OpenGLBackend::toGLFilter(TextureFilter f) const {
#ifdef FELISS_HAS_GLAD
    switch(f) {
        case TextureFilter::Nearest:     return GL_NEAREST;
        case TextureFilter::Trilinear:   return GL_LINEAR_MIPMAP_LINEAR;
        case TextureFilter::Anisotropic: return GL_LINEAR_MIPMAP_LINEAR;
        default:                         return GL_LINEAR;
    }
#else
    return 0;
#endif
}
u32 OpenGLBackend::toGLWrap(TextureWrap w) const {
#ifdef FELISS_HAS_GLAD
    switch(w) {
        case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
        case TextureWrap::Mirror: return GL_MIRRORED_REPEAT;
        default:                  return GL_REPEAT;
    }
#else
    return 0;
#endif
}
u32 OpenGLBackend::toGLFormat(PixelFormat f, bool* isDepth) const {
#ifdef FELISS_HAS_GLAD
    if (isDepth) *isDepth = false;
    switch(f) {
        case PixelFormat::RGBA8:            return GL_RGBA;
        case PixelFormat::RGB8:             return GL_RGB;
        case PixelFormat::RGBA16F:          return GL_RGBA;
        case PixelFormat::RGBA32F:          return GL_RGBA;
        case PixelFormat::R32F:             return GL_RED;
        case PixelFormat::RG32F:            return GL_RG;
        case PixelFormat::Depth24Stencil8:
            if (isDepth) *isDepth = true;  return GL_DEPTH_STENCIL;
        case PixelFormat::Depth32F:
            if (isDepth) *isDepth = true;  return GL_DEPTH_COMPONENT;
        default:                            return GL_RGBA;
    }
#else
    return 0;
#endif
}
u32 OpenGLBackend::toGLInternal(PixelFormat f) const {
#ifdef FELISS_HAS_GLAD
    switch(f) {
        case PixelFormat::RGBA8:           return GL_RGBA8;
        case PixelFormat::RGB8:            return GL_RGB8;
        case PixelFormat::RGBA16F:         return GL_RGBA16F;
        case PixelFormat::RGBA32F:         return GL_RGBA32F;
        case PixelFormat::R32F:            return GL_R32F;
        case PixelFormat::RG32F:           return GL_RG32F;
        case PixelFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        case PixelFormat::Depth32F:        return GL_DEPTH_COMPONENT32F;
        default:                           return GL_RGBA8;
    }
#else
    return 0;
#endif
}
u32 OpenGLBackend::toGLType(PixelFormat f) const {
#ifdef FELISS_HAS_GLAD
    switch(f) {
        case PixelFormat::RGBA16F:
        case PixelFormat::RGB16F:          return GL_HALF_FLOAT;
        case PixelFormat::RGBA32F:
        case PixelFormat::R32F:
        case PixelFormat::RG32F:           return GL_FLOAT;
        case PixelFormat::Depth24Stencil8: return GL_UNSIGNED_INT_24_8;
        case PixelFormat::Depth32F:        return GL_FLOAT;
        default:                           return GL_UNSIGNED_BYTE;
    }
#else
    return 0;
#endif
}

GpuHandle OpenGLBackend::createTexture2D(int w, int h, PixelFormat fmt, const void* data,
                                          TextureFilter filter, TextureWrap wrap) {
#ifdef FELISS_HAS_GLAD
    bool isDepth = false;
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0,
        (GLint)toGLInternal(fmt), w, h, 0,
        toGLFormat(fmt, &isDepth), toGLType(fmt), data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)toGLFilter(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        (filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)toGLWrap(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)toGLWrap(wrap));
    if (filter == TextureFilter::Trilinear || filter == TextureFilter::Anisotropic)
        glGenerateMipmap(GL_TEXTURE_2D);
    return static_cast<GpuHandle>(tex);
#else
    return NULL_GPU_HANDLE;
#endif
}

GpuHandle OpenGLBackend::createCubemap(int sz, PixelFormat fmt, const void* faces[6]) {
#ifdef FELISS_HAS_GLAD
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            (GLint)toGLInternal(fmt), sz, sz, 0,
            toGLFormat(fmt), toGLType(fmt), faces ? faces[i] : nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return static_cast<GpuHandle>(tex);
#else
    return NULL_GPU_HANDLE;
#endif
}

void OpenGLBackend::bindTexture(GpuHandle tex, u32 slot) {
#ifdef FELISS_HAS_GLAD
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tex));
#endif
}

void OpenGLBackend::updateTexture2D(GpuHandle tex, int x, int y, int w, int h, const void* d) {
#ifdef FELISS_HAS_GLAD
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tex));
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, d);
#endif
}

void OpenGLBackend::destroyTexture(GpuHandle tex) {
#ifdef FELISS_HAS_GLAD
    GLuint t = static_cast<GLuint>(tex);
    if (t) glDeleteTextures(1, &t);
#endif
}

// ---- Framebuffers ----
GpuHandle OpenGLBackend::createFramebuffer(int w, int h, PixelFormat fmt, bool hasDepth) {
#ifdef FELISS_HAS_GLAD
    FBData fb;
    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    // Color attachment
    glGenTextures(1, &fb.colorTex);
    glBindTexture(GL_TEXTURE_2D, fb.colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)toGLInternal(fmt), w, h, 0,
                 toGLFormat(fmt), toGLType(fmt), nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fb.colorTex, 0);

    // Depth-stencil attachment
    if (hasDepth) {
        glGenTextures(1, &fb.depthTex);
        glBindTexture(GL_TEXTURE_2D, fb.depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0,
                     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, fb.depthTex, 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        FLS_ERRORF("OpenGL", "Framebuffer incomplete: 0x" << std::hex << status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GpuHandle h = static_cast<GpuHandle>(fb.fbo);
    m_fbData[h] = fb;
    return h;
#else
    return NULL_GPU_HANDLE;
#endif
}

void OpenGLBackend::bindFramebuffer(GpuHandle fb) {
#ifdef FELISS_HAS_GLAD
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fb));
#endif
}

GpuHandle OpenGLBackend::framebufferColorTexture(GpuHandle fb) {
    auto it = m_fbData.find(fb);
    return (it != m_fbData.end()) ? static_cast<GpuHandle>(it->second.colorTex) : NULL_GPU_HANDLE;
}

GpuHandle OpenGLBackend::framebufferDepthTexture(GpuHandle fb) {
    auto it = m_fbData.find(fb);
    return (it != m_fbData.end()) ? static_cast<GpuHandle>(it->second.depthTex) : NULL_GPU_HANDLE;
}

void OpenGLBackend::blitFramebuffer(GpuHandle src, GpuHandle dst,
                                     int sw, int sh, int dw, int dh) {
#ifdef FELISS_HAS_GLAD
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(src));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(dst));
    glBlitFramebuffer(0,0,sw,sh, 0,0,dw,dh,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void OpenGLBackend::destroyFramebuffer(GpuHandle fb) {
#ifdef FELISS_HAS_GLAD
    auto it = m_fbData.find(fb);
    if (it != m_fbData.end()) {
        auto& d = it->second;
        if (d.colorTex) glDeleteTextures(1, &d.colorTex);
        if (d.depthTex) glDeleteTextures(1, &d.depthTex);
        glDeleteFramebuffers(1, &d.fbo);
        m_fbData.erase(it);
    }
#endif
}

// ---- Draw ----
void OpenGLBackend::drawArrays(u32 mode, int first, int count) {
#ifdef FELISS_HAS_GLAD
    glDrawArrays(mode, first, count);
#endif
}
void OpenGLBackend::drawIndexed(u32 mode, int count, bool u32Idx) {
#ifdef FELISS_HAS_GLAD
    glDrawElements(mode, count,
        u32Idx ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, nullptr);
#endif
}
void OpenGLBackend::drawArraysInstanced(u32 mode, int first, int count, int inst) {
#ifdef FELISS_HAS_GLAD
    glDrawArraysInstanced(mode, first, count, inst);
#endif
}
void OpenGLBackend::drawIndexedInstanced(u32 mode, int cnt, int inst, bool u32Idx) {
#ifdef FELISS_HAS_GLAD
    glDrawElementsInstanced(mode, cnt,
        u32Idx ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, nullptr, inst);
#endif
}

// ---- Compute ----
GpuHandle OpenGLBackend::createComputeShader(const char* src) {
    return createShader(ShaderStage::Compute, src, true);
}
void OpenGLBackend::dispatchCompute(GpuHandle cs, u32 x, u32 y, u32 z) {
#ifdef FELISS_HAS_GLAD
    glUseProgram(static_cast<GLuint>(cs));
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
#endif
}

// ---- ImGui ----
void OpenGLBackend::imguiInit() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI) && defined(FELISS_HAS_GLFW)
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_glfwWindow), true);
    ImGui_ImplOpenGL3_Init("#version 450");
    FLS_INFO("OpenGL", "ImGui OpenGL3 backend initialized");
#endif
}
void OpenGLBackend::imguiNewFrame() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
#endif
}
void OpenGLBackend::imguiRender() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}
void OpenGLBackend::imguiShutdown() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

// ---- Device info ----
std::string OpenGLBackend::deviceName() const {
#ifdef FELISS_HAS_GLAD
    const char* s = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    return s ? s : "Unknown";
#else
    return "No GLAD";
#endif
}
std::string OpenGLBackend::driverVersion() const {
#ifdef FELISS_HAS_GLAD
    const char* s = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    return s ? s : "";
#else
    return "";
#endif
}
int OpenGLBackend::maxTextureSize() const {
#ifdef FELISS_HAS_GLAD
    GLint v = 4096;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &v);
    return v;
#else
    return 4096;
#endif
}

} // namespace Feliss

#endif // FELISS_RENDERER_OPENGL
