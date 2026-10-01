cmake_minimum_required(VERSION 3.16)

project(dash VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DASH_ENABLE_WARNINGS "Enable warnings" ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG" CACHE STRING "C++ release flags" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG" CACHE STRING "C release flags" FORCE)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C++ release flags" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C release flags" FORCE)
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

set(IMGUI_UI_BUILD_RAYLIB ON)
set(IMGUI_UI_BUILD_TESTS ON)

find_package(OpenGL REQUIRED)

add_subdirectory(vendor/imgui-ui)
add_subdirectory(vendor/box2d)
add_subdirectory(vendor/json)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.hpp")

if(DASH_ENABLE_WARNINGS)
    if(MSVC)
        set(DASH_WARNING_FLAGS /W4 /permissive-)
    else()
        set(DASH_WARNING_FLAGS
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
        )
    endif()

    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_OPTIONS "${DASH_WARNING_FLAGS}")
endif()

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui-ui::raylib
    box2d::box2d
    nlohmann_json
    OpenGL::GL
)
