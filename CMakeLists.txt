# Top-level CMakeLists
cmake_minimum_required(VERSION 3.18)
project(FelissEngine VERSION 1.0.0 LANGUAGES C CXX)

# Global Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DUNICODE -D_UNICODE)

# Platform Options
option(ENABLE_PS5 "Enable PS5 Platform" OFF)
option(ENABLE_XBOX "Enable Xbox Platform" OFF)
option(ENABLE_ANDROID "Enable Android" OFF)
option(ENABLE_IOS "Enable iOS" OFF)

# ThirdParty Libraries
add_subdirectory(ThirdParty/AsterCore)
add_subdirectory(ThirdParty/FMOD)
add_subdirectory(ThirdParty/ImGui)

# Engine Modules
add_subdirectory(Engine/Core)
add_subdirectory(Engine/Render)
add_subdirectory(Engine/Scene)
add_subdirectory(Engine/Physics)
add_subdirectory(Engine/Script)
add_subdirectory(Engine/AI)
add_subdirectory(Engine/Audio)
add_subdirectory(Engine/UI)

# Runtime
add_subdirectory(Runtime)

# Editor Tools
add_subdirectory(Editor)
add_subdirectory(Tools/LevelEditor)

# Samples
add_subdirectory(Samples)
add_subdirectory(Launcher)

# Install rules
include(GNUInstallDirs)
install(DIRECTORY Engine/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ThirdParty/ DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Global Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DUNICODE -D_UNICODE)
# TODO: Consider using target_compile_definitions instead of global add_definitions for better scope control.

# ThirdParty Libraries
add_subdirectory(ThirdParty/ImGui)
# FIXME: Check whether ImGui is properly linked only where needed, to avoid unnecessary coupling.

# Install rules
install(DIRECTORY Engine/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ThirdParty/ DESTINATION ${CMAKE_INSTALL_LIBDIR})
# TODO: Add fine-grained install(TARGETS ...) commands for better packaging and dependency control.

# Editor Tools
add_subdirectory(Editor)
add_subdirectory(Tools/LevelEditor)
# TODO: Consider making Editor optional via a CMake option to allow minimal runtime builds.
