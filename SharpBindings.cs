using System;
using System.Runtime.InteropServices;

namespace FelissEngine {

    public enum AntiAliasingMode {
        NONE,
        FXAA,
        SMAA,
        TAA,
        DLAA
    }

    public static class Renderer {
        [DllImport("FelissRenderer", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetAntiAliasingMode(AntiAliasingMode mode);

        [DllImport("FelissRenderer", CallingConvention = CallingConvention.Cdecl)]
        public static extern AntiAliasingMode GetAntiAliasingMode();
    }

    public static class SceneSystem {
        [DllImport("FelissScene", CallingConvention = CallingConvention.Cdecl)]
        public static extern void LoadScene(string path);

        [DllImport("FelissScene", CallingConvention = CallingConvention.Cdecl)]
        public static extern void UpdateScene(float deltaTime);
    }

    public static class Physics {
        [DllImport("FelissPhysics", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Simulate(float deltaTime);
    }

    public static class Input {
        [DllImport("FelissRuntime", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool IsKeyPressed(int keycode);
    }

    public static class Logger {
        [DllImport("FelissCore", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Log(string msg);
    }
}
