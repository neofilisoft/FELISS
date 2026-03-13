#include "platform/Window.h"
#include "core/Logger.h"

#ifdef FELISS_HAS_GLFW
#  include <GLFW/glfw3.h>
#endif

namespace Feliss {

// ---- GLFW error callback ----
#ifdef FELISS_HAS_GLFW
static void glfwErrorCb(int code, const char* desc) {
    FLS_ERRORF("GLFW", "Error " << code << ": " << desc);
}
#endif

Window::Window(const WindowDesc& desc)
    : m_desc(desc), m_width(desc.width), m_height(desc.height) {}

Window::~Window() { shutdown(); }

bool Window::init() {
#ifdef FELISS_HAS_GLFW
    glfwSetErrorCallback(glfwErrorCb);
    if (!glfwInit()) {
        FLS_ERROR("Window", "glfwInit failed");
        return false;
    }

    // OpenGL hints (override in VulkanBackend init if needed)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, m_desc.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

    GLFWmonitor* monitor = m_desc.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_handle = glfwCreateWindow(m_width, m_height,
                                m_desc.title.c_str(), monitor, nullptr);
    if (!m_handle) {
        FLS_ERROR("Window", "glfwCreateWindow failed");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_handle));
    glfwSwapInterval(m_desc.vsync ? 1 : 0);

    // Store this pointer for callbacks
    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_handle), this);

    // Register GLFW callbacks
    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(m_handle),
        [](GLFWwindow* w, int wi, int hi) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
            self->m_width = wi; self->m_height = hi;
            if (self->m_resizeCb) self->m_resizeCb(wi, hi);
        });

    glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(m_handle),
        [](GLFWwindow* w) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (self->m_closeCb) self->m_closeCb();
        });

    glfwSetScrollCallback(static_cast<GLFWwindow*>(m_handle),
        [](GLFWwindow* w, double x, double y) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
            self->m_scrollAccum.x += static_cast<f32>(x);
            self->m_scrollAccum.y += static_cast<f32>(y);
        });

    glfwSetCursorPosCallback(static_cast<GLFWwindow*>(m_handle),
        [](GLFWwindow* w, double x, double y) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
            Vec2 cur = { static_cast<f32>(x), static_cast<f32>(y) };
            if (self->m_firstMouse) {
                self->m_lastMousePos = cur;
                self->m_firstMouse   = false;
            }
            self->m_mouseDelta = cur - self->m_lastMousePos;
            self->m_mousePos   = cur;
            self->m_lastMousePos = cur;
        });

    // Content scale (HiDPI)
    float xs = 1.0f, ys = 1.0f;
    glfwGetWindowContentScale(static_cast<GLFWwindow*>(m_handle), &xs, &ys);
    m_contentScale = (xs + ys) * 0.5f;

    FLS_INFOF("Window", "Window created: " << m_width << "x" << m_height
        << " scale=" << m_contentScale);
    return true;
#else
    FLS_ERROR("Window", "GLFW not compiled in");
    return false;
#endif
}

void Window::shutdown() {
#ifdef FELISS_HAS_GLFW
    if (m_handle) {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_handle));
        m_handle = nullptr;
        glfwTerminate();
        FLS_INFO("Window", "Window destroyed");
    }
#endif
}

void Window::pollEvents() {
#ifdef FELISS_HAS_GLFW
    m_scrollAccum = {0,0};
    m_mouseDelta  = {0,0};
    glfwPollEvents();
#endif
}

void Window::swapBuffers() {
#ifdef FELISS_HAS_GLFW
    if (m_handle)
        glfwSwapBuffers(static_cast<GLFWwindow*>(m_handle));
#endif
}

bool Window::shouldClose() const {
#ifdef FELISS_HAS_GLFW
    return m_handle && glfwWindowShouldClose(static_cast<GLFWwindow*>(m_handle));
#else
    return true;
#endif
}

void Window::setTitle(const std::string& t) {
#ifdef FELISS_HAS_GLFW
    if (m_handle) glfwSetWindowTitle(static_cast<GLFWwindow*>(m_handle), t.c_str());
#endif
}

void Window::setSize(int w, int h) {
    m_width = w; m_height = h;
#ifdef FELISS_HAS_GLFW
    if (m_handle) glfwSetWindowSize(static_cast<GLFWwindow*>(m_handle), w, h);
#endif
}

void Window::setFullscreen(bool full) {
#ifdef FELISS_HAS_GLFW
    if (!m_handle) return;
    if (full) {
        GLFWmonitor* mon = glfwGetPrimaryMonitor();
        const GLFWvidmode* vm = glfwGetVideoMode(mon);
        glfwSetWindowMonitor(static_cast<GLFWwindow*>(m_handle),
                             mon, 0, 0, vm->width, vm->height, vm->refreshRate);
    } else {
        glfwSetWindowMonitor(static_cast<GLFWwindow*>(m_handle),
                             nullptr, 100, 100, m_width, m_height, 0);
    }
#endif
}

bool Window::rawKey(KeyCode k) const {
#ifdef FELISS_HAS_GLFW
    if (!m_handle) return false;
    return glfwGetKey(static_cast<GLFWwindow*>(m_handle), static_cast<int>(k)) == GLFW_PRESS;
#else
    return false;
#endif
}

bool Window::rawMouseButton(MouseButton b) const {
#ifdef FELISS_HAS_GLFW
    if (!m_handle) return false;
    return glfwGetMouseButton(static_cast<GLFWwindow*>(m_handle), static_cast<int>(b)) == GLFW_PRESS;
#else
    return false;
#endif
}

Vec2 Window::rawMousePos()    const { return m_mousePos; }
Vec2 Window::rawMouseScroll() const { return m_scrollAccum; }

} // namespace Feliss
