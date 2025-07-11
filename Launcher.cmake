# FelissEngine/Launcher/CMakeLists

cmake_minimum_required(VERSION 3.18)
project(FelissLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include engine headers
include_directories(
    ${CMAKE_SOURCE_DIR}/Engine/Core/include
    ${CMAKE_SOURCE_DIR}/Engine/Render/include
    ${CMAKE_SOURCE_DIR}/Engine/Scene/include
    ${CMAKE_SOURCE_DIR}/Engine/Physics/include
    ${CMAKE_SOURCE_DIR}/Engine/Script/include
    ${CMAKE_SOURCE_DIR}/Runtime
)

# Source file
add_executable(FelissLauncher launcher.cpp)

# Link with engine modules (assumes those are built as static/shared libraries)
target_link_libraries(FelissLauncher
    FelissCore
    FelissRenderer
    FelissScene
    FelissPhysics
    FelissScript
)

# Post-build copy configuration.ini
add_custom_command(TARGET FelissLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/configuration.ini
        $<TARGET_FILE_DIR:FelissLauncher>/configuration.ini
)
# FelissEngine/Launcher/CMakeLists.txt

add_executable(FelissLauncher main.cpp)

target_include_directories(FelissLauncher PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine/Core/include
    ${CMAKE_SOURCE_DIR}/Engine/Render/include
    ${CMAKE_SOURCE_DIR}/Engine/Scene/include
    ${CMAKE_SOURCE_DIR}/Engine/Physics/include
    ${CMAKE_SOURCE_DIR}/Runtime
)

target_link_libraries(FelissLauncher
    FelissCore
    FelissRenderer
    FelissScene
    FelissPhysics
)
