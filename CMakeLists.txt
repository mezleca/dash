cmake_minimum_required(VERSION 3.16)

project(dash VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)

find_package(OpenGL REQUIRED)

add_subdirectory(vendor/raylib)
add_subdirectory(vendor/json)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB IMGUI_CORE_SOURCES "vendor/imgui/*.cpp")

add_executable(${PROJECT_NAME}
    vendor/rlimgui/rlImGui.cpp
    vendor/stb/stb_image.h
    ${IMGUI_CORE_SOURCES}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    vendor/rlimgui
    vendor/imgui/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    nlohmann_json
    OpenGL::GL
)
