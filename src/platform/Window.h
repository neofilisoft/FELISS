#pragma once
#include "feliss/Types.h"
#include "core/EventSystem.h"
#include <string>
#include <functional>

namespace Feliss {

struct WindowDesc {
    std::string title  = "Feliss Engine";
    int         width  = 1280;
    int         height = 720;
    bool        fullscreen = false;
    bool        vsync      = true;
    bool        resizable  = true;
};

class Window {
public:
    explicit Window(const WindowDesc& desc);
    ~Window();
    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    bool  init();
    void  shutdown();
    void  pollEvents();
    void  swapBuffers();
    bool  shouldClose() const;
    void  setTitle(const std::string& t);
    void  setSize(int w, int h);
    void  setFullscreen(bool full);

    int   width()  const { return m_width; }
    int   height() const { return m_height; }
    void* nativeHandle() const { return m_handle; }   // GLFWwindow*
    float contentScale() const { return m_contentScale; }

    // Event callbacks (set by Engine)
    using ResizeCb = std::function<void(int,int)>;
    using CloseCb  = std::function<void()>;
    void setResizeCallback(ResizeCb cb) { m_resizeCb = std::move(cb); }
    void setCloseCallback(CloseCb cb)   { m_closeCb  = std::move(cb); }

    // Input state queries (raw GLFW, before InputSystem processes)
    bool rawKey(KeyCode k)         const;
    bool rawMouseButton(MouseButton b) const;
    Vec2 rawMousePos()             const;
    Vec2 rawMouseScroll()          const;

private:
    static void cbResize(void* win, int w, int h);
    static void cbClose(void* win);
    static void cbKey(void* win, int key, int sc, int action, int mods);
    static void cbMouseBtn(void* win, int btn, int action, int mods);
    static void cbMousePos(void* win, double x, double y);
    static void cbScroll(void* win, double x, double y);
    static void cbChar(void* win, unsigned int cp);

    WindowDesc  m_desc;
    int         m_width  = 1280;
    int         m_height = 720;
    float       m_contentScale = 1.0f;
    void*       m_handle = nullptr;  // GLFWwindow*

    ResizeCb    m_resizeCb;
    CloseCb     m_closeCb;
    Vec2        m_scrollAccum = {0,0};
    Vec2        m_mousePos    = {0,0};
    Vec2        m_mouseDelta  = {0,0};
    Vec2        m_lastMousePos = {0,0};
    bool        m_firstMouse = true;
};

} // namespace Feliss
