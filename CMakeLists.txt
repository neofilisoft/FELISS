cmake_minimum_required(VERSION 3.20)
project(FelissEngine VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =====================================================================
# Options
# =====================================================================
option(FELISS_BUILD_EDITOR   "Build ImGui editor"          ON)
option(FELISS_RENDERER_OPENGL "Enable OpenGL renderer"     ON)
option(FELISS_RENDERER_VULKAN "Enable Vulkan renderer"     OFF)
option(FELISS_RENDERER_D3D11  "Enable DirectX 11"         OFF)
option(FELISS_RENDERER_D3D12  "Enable DirectX 12"         OFF)
option(FELISS_RENDERER_METAL  "Enable Metal"              OFF)
option(FELISS_LUA_ENABLED    "Enable Lua scripting"        ON)
option(FELISS_CSHARP_ENABLED "Enable C# scripting (Mono)" OFF)
option(FELISS_USE_JOLT       "Enable Jolt Physics"        OFF)
option(FELISS_USE_PHYSX      "Enable NVIDIA PhysX"        OFF)
option(FELISS_BUILD_TESTS    "Build unit tests"           OFF)

# =====================================================================
# Source files
# =====================================================================
set(FLS_SOURCES
    main.cpp
    src/core/Logger.cpp
    src/core/Timer.cpp
    src/core/EventSystem.cpp
    src/core/Engine.cpp
    src/ecs/World.cpp
    src/renderer/RenderPipeline.cpp
    src/renderer/backends/BackendFactory.cpp
    src/physics/PhysicsWorld.cpp
    src/audio/AudioEngine.cpp
    src/platform/Window.cpp
    src/scripting/ScriptEngine.cpp
    src/scripting/LuaBridge.cpp
    src/scripting/CSharpBridge.cpp
    src/editor/ExtensionManager.cpp
    src/api/feliss_api.cpp
)

if(FELISS_RENDERER_OPENGL)
    list(APPEND FLS_SOURCES src/renderer/backends/OpenGLBackend.cpp)
endif()

if(FELISS_BUILD_EDITOR)
    list(APPEND FLS_SOURCES src/editor/EditorApp.cpp)
endif()

# =====================================================================
# Executable
# =====================================================================
add_executable(FelissEngine ${FLS_SOURCES})

target_include_directories(FelissEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/entt/single_include
)

# =====================================================================
# Compile definitions
# =====================================================================
if(FELISS_BUILD_EDITOR)
    target_compile_definitions(FelissEngine PRIVATE FELISS_EDITOR_BUILD)
endif()
if(FELISS_RENDERER_OPENGL)
    target_compile_definitions(FelissEngine PRIVATE FELISS_RENDERER_OPENGL)
endif()
if(FELISS_RENDERER_VULKAN)
    target_compile_definitions(FelissEngine PRIVATE FELISS_RENDERER_VULKAN)
endif()
if(FELISS_RENDERER_D3D11)
    target_compile_definitions(FelissEngine PRIVATE FELISS_RENDERER_D3D11)
endif()
if(FELISS_RENDERER_D3D12)
    target_compile_definitions(FelissEngine PRIVATE FELISS_RENDERER_D3D12)
endif()
if(FELISS_RENDERER_METAL)
    target_compile_definitions(FelissEngine PRIVATE FELISS_RENDERER_METAL)
endif()
if(FELISS_LUA_ENABLED)
    target_compile_definitions(FelissEngine PRIVATE FELISS_LUA_ENABLED)
endif()
if(FELISS_CSHARP_ENABLED)
    target_compile_definitions(FelissEngine PRIVATE FELISS_CSHARP_ENABLED)
endif()

# =====================================================================
# GLFW
# =====================================================================
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glfw)
if(EXISTS "${GLFW_DIR}/CMakeLists.txt")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${GLFW_DIR} ${CMAKE_BINARY_DIR}/glfw)
    target_link_libraries(FelissEngine PRIVATE glfw)
    target_include_directories(FelissEngine PRIVATE ${GLFW_DIR}/include)
    target_compile_definitions(FelissEngine PRIVATE FELISS_HAS_GLFW)
else()
    find_package(glfw3 QUIET)
    if(glfw3_FOUND)
        target_link_libraries(FelissEngine PRIVATE glfw)
        target_compile_definitions(FelissEngine PRIVATE FELISS_HAS_GLFW)
        message(STATUS "GLFW found via find_package")
    else()
        message(WARNING "GLFW not found — window creation will be disabled")
    endif()
endif()

# =====================================================================
# GLAD (OpenGL)
# =====================================================================
if(FELISS_RENDERER_OPENGL)
    set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glad)
    if(EXISTS "${GLAD_DIR}/src/glad.c")
        add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
        target_include_directories(glad PUBLIC ${GLAD_DIR}/include)
        target_link_libraries(FelissEngine PRIVATE glad)
        target_compile_definitions(FelissEngine PRIVATE FELISS_HAS_GLAD)
    else()
        find_package(OpenGL QUIET)
        if(OpenGL_FOUND)
            target_link_libraries(FelissEngine PRIVATE OpenGL::GL)
            message(STATUS "OpenGL found (no GLAD — provide your own loader)")
        else()
            message(WARNING "OpenGL not found")
        endif()
    endif()
endif()

# =====================================================================
# ImGui
# =====================================================================
if(FELISS_BUILD_EDITOR)
    set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui)
    if(EXISTS "${IMGUI_DIR}/imgui.cpp")
        add_library(imgui STATIC
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/imgui_demo.cpp
            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )
        target_include_directories(imgui PUBLIC
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
            ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glfw/include
        )
        target_link_libraries(FelissEngine PRIVATE imgui)
        target_compile_definitions(FelissEngine PRIVATE FELISS_HAS_IMGUI)
    else()
        message(WARNING "ImGui source not found — editor UI disabled (copy imgui/ into third-party/imgui/)")
    endif()
endif()

# =====================================================================
# Lua 5.4
# =====================================================================
if(FELISS_LUA_ENABLED)
    set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/lua)
    file(GLOB LUA_SOURCES "${LUA_DIR}/*.c")
    # Exclude luac (compiler) and lua (interpreter) main files
    list(FILTER LUA_SOURCES EXCLUDE REGEX ".*(luac|lua)\\.c$")
    if(LUA_SOURCES)
        add_library(lua54 STATIC ${LUA_SOURCES})
        target_include_directories(lua54 PUBLIC ${LUA_DIR})
        target_link_libraries(FelissEngine PRIVATE lua54)
        if(UNIX AND NOT APPLE)
            target_link_libraries(lua54 PUBLIC m dl)
        endif()
    else()
        find_package(Lua QUIET)
        if(Lua_FOUND)
            target_include_directories(FelissEngine PRIVATE ${LUA_INCLUDE_DIR})
            target_link_libraries(FelissEngine PRIVATE ${LUA_LIBRARIES})
        else()
            message(WARNING "Lua source not found — copy Lua 5.4 source into third-party/lua/")
            target_compile_definitions(FelissEngine PRIVATE -UFELISS_LUA_ENABLED)
        endif()
    endif()
endif()

# =====================================================================
# Mono / C#
# =====================================================================
if(FELISS_CSHARP_ENABLED)
    find_path(MONO_INCLUDE_DIR mono/jit/jit.h PATHS /usr/include/mono-2.0 /usr/local/include/mono-2.0)
    find_library(MONO_LIB NAMES mono-2.0 monosgen-2.0 PATHS /usr/lib /usr/local/lib)
    if(MONO_INCLUDE_DIR AND MONO_LIB)
        target_include_directories(FelissEngine PRIVATE ${MONO_INCLUDE_DIR})
        target_link_libraries(FelissEngine PRIVATE ${MONO_LIB})
    else()
        message(WARNING "Mono not found — C# scripting disabled")
        target_compile_definitions(FelissEngine PRIVATE -UFELISS_CSHARP_ENABLED)
    endif()
endif()

# =====================================================================
# Jolt Physics
# =====================================================================
if(FELISS_USE_JOLT)
    set(JOLT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/jolt)
    if(EXISTS "${JOLT_DIR}/CMakeLists.txt")
        add_subdirectory(${JOLT_DIR} ${CMAKE_BINARY_DIR}/jolt)
        target_link_libraries(FelissEngine PRIVATE Jolt)
        target_compile_definitions(FelissEngine PRIVATE FELISS_HAS_JOLT)
    else()
        message(WARNING "Jolt not found — copy Jolt Physics into third-party/jolt/")
    endif()
endif()

# =====================================================================
# Platform-specific
# =====================================================================
if(WIN32)
    target_link_libraries(FelissEngine PRIVATE ws2_32)
    if(FELISS_RENDERER_D3D11)
        target_link_libraries(FelissEngine PRIVATE d3d11 dxgi d3dcompiler)
    endif()
    if(FELISS_RENDERER_D3D12)
        target_link_libraries(FelissEngine PRIVATE d3d12 dxgi d3dcompiler)
    endif()
elseif(APPLE)
    if(FELISS_RENDERER_METAL)
        target_link_libraries(FelissEngine PRIVATE "-framework Metal" "-framework MetalKit" "-framework Cocoa")
    endif()
elseif(UNIX)
    target_link_libraries(FelissEngine PRIVATE pthread dl)
endif()

# =====================================================================
# Extensions (auto-discover)
# =====================================================================
file(GLOB EXT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/extensions/*/CMakeLists.txt")
foreach(EXT_CMAKE ${EXT_DIRS})
    get_filename_component(EXT_DIR ${EXT_CMAKE} DIRECTORY)
    message(STATUS "Found extension: ${EXT_DIR}")
    add_subdirectory(${EXT_DIR} ${CMAKE_BINARY_DIR}/ext/${EXT_DIR})
endforeach()

# =====================================================================
# C ABI shared library (for C#/Rust/Zig bindings)
# =====================================================================
add_library(feliss_core SHARED
    src/api/feliss_api.cpp
    src/core/Logger.cpp
    src/core/Timer.cpp
    src/core/EventSystem.cpp
    src/core/Engine.cpp
    src/ecs/World.cpp
    src/renderer/RenderPipeline.cpp
    src/renderer/backends/BackendFactory.cpp
    src/physics/PhysicsWorld.cpp
    src/audio/AudioEngine.cpp
    src/platform/Window.cpp
    src/scripting/ScriptEngine.cpp
    src/scripting/LuaBridge.cpp
    src/scripting/CSharpBridge.cpp
    src/editor/ExtensionManager.cpp
)
if(FELISS_RENDERER_OPENGL)
    target_sources(feliss_core PRIVATE src/renderer/backends/OpenGLBackend.cpp)
endif()
target_compile_definitions(feliss_core PRIVATE FELISS_EXPORT)
target_include_directories(feliss_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glm
)
# Pass same flags as main target
get_target_property(FLS_DEFS FelissEngine COMPILE_DEFINITIONS)
if(FLS_DEFS)
    target_compile_definitions(feliss_core PRIVATE ${FLS_DEFS})
endif()

# =====================================================================
# Compiler warnings
# =====================================================================
if(MSVC)
    target_compile_options(FelissEngine PRIVATE /W4 /wd4100 /wd4189)
else()
    target_compile_options(FelissEngine PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

message(STATUS "")
message(STATUS "=== Feliss Engine Build Configuration ===")
message(STATUS " Editor:    ${FELISS_BUILD_EDITOR}")
message(STATUS " OpenGL:    ${FELISS_RENDERER_OPENGL}")
message(STATUS " Vulkan:    ${FELISS_RENDERER_VULKAN}")
message(STATUS " D3D11:     ${FELISS_RENDERER_D3D11}")
message(STATUS " D3D12:     ${FELISS_RENDERER_D3D12}")
message(STATUS " Metal:     ${FELISS_RENDERER_METAL}")
message(STATUS " Lua:       ${FELISS_LUA_ENABLED}")
message(STATUS " C#/Mono:   ${FELISS_CSHARP_ENABLED}")
message(STATUS " Jolt:      ${FELISS_USE_JOLT}")
message(STATUS " PhysX:     ${FELISS_USE_PHYSX}")
message(STATUS "==========================================")
