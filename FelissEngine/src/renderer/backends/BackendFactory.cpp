#include "renderer/IRendererBackend.h"
#include "renderer/backends/OpenGLBackend.h"
#include "renderer/backends/VulkanBackend.h"
#include "renderer/backends/D3D11Backend.h"
#include "renderer/backends/D3D12Backend.h"
#include "renderer/backends/MetalBackend.h"
#include "core/Logger.h"

namespace Feliss {

// =====================================================================
// Factory — choose backend at runtime
// =====================================================================
Scope<IRendererBackend> createRendererBackend(RenderAPI api) {
    switch (api) {
#ifdef FELISS_RENDERER_OPENGL
        case RenderAPI::OpenGL:
            FLS_INFO("Renderer", "Creating OpenGL backend");
            return MakeScope<OpenGLBackend>();
#endif
#ifdef FELISS_RENDERER_VULKAN
        case RenderAPI::Vulkan:
            FLS_INFO("Renderer", "Creating Vulkan backend");
            return MakeScope<VulkanBackend>();
#endif
#ifdef FELISS_RENDERER_D3D11
        case RenderAPI::DirectX11:
            FLS_INFO("Renderer", "Creating DirectX 11 backend");
            return MakeScope<D3D11Backend>();
#endif
#ifdef FELISS_RENDERER_D3D12
        case RenderAPI::DirectX12:
            FLS_INFO("Renderer", "Creating DirectX 12 backend");
            return MakeScope<D3D12Backend>();
#endif
#ifdef FELISS_RENDERER_METAL
        case RenderAPI::Metal:
            FLS_INFO("Renderer", "Creating Metal backend");
            return MakeScope<MetalBackend>();
#endif
        default:
            FLS_WARNF("Renderer", "API " << RenderAPIToString(api) <<
                " not compiled in — falling back to OpenGL");
            return MakeScope<OpenGLBackend>();
    }
}

// =====================================================================
// Stub init/shutdown for non-OpenGL backends
// These are empty stubs — real implementations go in separate files.
// =====================================================================
bool VulkanBackend::init(Window&) {
    FLS_WARN("Vulkan", "Vulkan backend is a stub — not yet implemented");
    return false;
}
void VulkanBackend::shutdown() {}
void VulkanBackend::imguiInit()     {}
void VulkanBackend::imguiNewFrame() {}
void VulkanBackend::imguiRender()   {}
void VulkanBackend::imguiShutdown() {}

bool D3D11Backend::init(Window&) {
    FLS_WARN("D3D11", "DirectX 11 backend is a stub — Windows SDK required");
    return false;
}
void D3D11Backend::shutdown() {}

bool D3D12Backend::init(Window&) {
    FLS_WARN("D3D12", "DirectX 12 backend is a stub — Windows SDK required");
    return false;
}
void D3D12Backend::shutdown() {}

bool MetalBackend::init(Window&) {
    FLS_WARN("Metal", "Metal backend is a stub — macOS/Xcode required");
    return false;
}
void MetalBackend::shutdown() {}

} // namespace Feliss
