include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Windowing & Input - SDL3
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG release-3.4.20
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# Rendering - BGFX
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)

if(ANDROID)
    set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
else()
    set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(
        bgfx
        GIT_REPOSITORY https://github.com/bkaradzic/bgfx
        GIT_TAG master
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(bgfx)

# 3D Physics - Jolt Physics
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY https://github.com/jrouwe/joltphysics
        GIT_TAG v5.7.0
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR Build
)
FetchContent_MakeAvailable(JoltPhysics)

# 2D Physics - Box2D
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d
        GIT_TAG v3.2.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(box2d)

# Maths - GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# STB Libraries (Images/Audio/Font Loading)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb
        GIT_TAG master
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(stb)
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE "${stb_SOURCE_DIR}")

# Debug GUI - Dear ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.93.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_imp_sdl3.cpp"
)

target_include_directories(imgui PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)