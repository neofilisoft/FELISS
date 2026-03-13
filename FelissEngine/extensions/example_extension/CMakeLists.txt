cmake_minimum_required(VERSION 3.20)
project(example_extension)

add_library(example_extension SHARED example_extension.cpp)

target_include_directories(example_extension PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(example_extension PROPERTIES
    PREFIX ""
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(WIN32)
    target_compile_definitions(example_extension PRIVATE FELISS_EXPORT)
endif()
