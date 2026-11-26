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

# Rendering - BGFX w/ GENie
include(ExternalProject)

set(BGFX_ROOT_DIR "${CMAKE_BINARY_DIR}/_deps/bgfx_upstream")
set(BX_DIR "${BGFX_ROOT_DIR}/bx")
set(BIMG_DIR "${BGFX_ROOT_DIR}/bimg")
set(BGFX_DIR "${BGFX_ROOT_DIR}/bgfx")

ExternalProject_Add(upstream_bx
        GIT_REPOSITORY https://github.com/bkaradzic/bx.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        SOURCE_DIR "${BX_DIR}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

ExternalProject_Add(upstream_bimg
        DEPENDS upstream_bx
        GIT_REPOSITORY https://github.com/bkaradzic/bimg.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        SOURCE_DIR "${BIMG_DIR}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

set(BGFX_OUT_LIBS "")

if(ANDROID)
    set(BGFX_GEN_DIR "${BGFX_DIR}/.build/projects/gmake-android-arm64")
    set(BGFX_OUT_DIR "${BGFX_DIR}/.build/android-arm64/bin")
    set(BGFX_LIB_PREFIX "lib")
    set(BGFX_LIB_EXT ".a")
    set(BGFX_CONFIG_SUFFIX "Release")

    if(CMAKE_HOST_WIN32)
        set(GENIE_EXE "${BX_DIR}/tools/bin/windows/genie.exe")
        set(NDK_MAKE_EXE "${CMAKE_ANDROID_NDK}/prebuilt/windows-x86_64/bin/make.exe")
    elseif(CMAKE_HOST_APPLE)
        set(GENIE_EXE "${BX_DIR}/tools/bin/darwin/genie")
        set(NDK_MAKE_EXE "make")
    else()
        set(GENIE_EXE "${BX_DIR}/tools/bin/linux/genie")
        set(NDK_MAKE_EXE "make")
    endif()

    set(BGFX_GENIE_CMD "${GENIE_EXE}" --with-android=24 --gcc=android-arm64 gmake)
    set(BGFX_BUILD_CMD ${CMAKE_COMMAND} -E env "ANDROID_NDK_ROOT=${CMAKE_ANDROID_NDK}" "${NDK_MAKE_EXE}" -C "${BGFX_GEN_DIR}" config=release64 bgfx bimg bx -j8)
elseif(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    get_filename_component(MINGW_ROOT_DIR "${MINGW_BIN_DIR}" DIRECTORY)

    set(BGFX_GEN_DIR "${BGFX_DIR}/.build/projects/gmake-mingw-gcc")
    set(BGFX_OUT_DIR "${BGFX_DIR}/.build/win64_mingw-gcc/bin")
    set(BGFX_LIB_PREFIX "lib")
    set(BGFX_LIB_EXT ".a")
    set(BGFX_CONFIG_SUFFIX "Release")

    set(BGFX_GENIE_CMD "${BX_DIR}/tools/bin/windows/genie.exe" --gcc=mingw-gcc --with-tools gmake)
    set(BGFX_BUILD_CMD "${CMAKE_MAKE_PROGRAM}" -C "${BGFX_GEN_DIR}"
            MINGW="${MINGW_ROOT_DIR}"
            CC="${CMAKE_C_COMPILER}"
            CXX="${CMAKE_CXX_COMPILER}"
            AR="${CMAKE_AR}"
            config=release64
            bgfx bimg bx
    )
else()
    # Visual Studio MSVC build via GENie -> MSBuild
    set(BGFX_GEN_DIR "${BGFX_DIR}/.build/projects/vs2022")
    set(BGFX_OUT_DIR "${BGFX_DIR}/.build/win64_vs2022/bin")
    set(BGFX_LIB_PREFIX "")
    set(BGFX_LIB_EXT ".lib")
    set(BGFX_CONFIG_SUFFIX "Release")

    set(BGFX_GENIE_CMD "${BX_DIR}/tools/bin/windows/genie.exe" --with-windows=10.0 --with-tools vs2022)
    set(BGFX_BUILD_CMD msbuild "${BGFX_GEN_DIR}/bgfx.sln" /p:Configuration=Release /p:Platform=x64 /t:bgfx;bimg;bx /m)
endif()

set(BGFX_OUT_LIBS
        "${BGFX_OUT_DIR}/${BGFX_LIB_PREFIX}bgfx${BGFX_CONFIG_SUFFIX}${BGFX_LIB_EXT}"
        "${BGFX_OUT_DIR}/${BGFX_LIB_PREFIX}bimg${BGFX_CONFIG_SUFFIX}${BGFX_LIB_EXT}"
        "${BGFX_OUT_DIR}/${BGFX_LIB_PREFIX}bx${BGFX_CONFIG_SUFFIX}${BGFX_LIB_EXT}"
)

ExternalProject_Add(upstream_bgfx_build
        DEPENDS upstream_bx upstream_bimg
        GIT_REPOSITORY https://github.com/bkaradzic/bgfx
        GIT_TAG master
        GIT_SHALLOW TRUE
        SOURCE_DIR "${BGFX_DIR}"
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${BGFX_GENIE_CMD}
        BUILD_COMMAND ${BGFX_BUILD_CMD}
        BUILD_BYPRODUCTS ${BGFX_OUT_LIBS}
        INSTALL_COMMAND ""
)

function(add_bgfx_imported_lib TARGET_NAME LIB_BASE_NAME)
    add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)
    add_dependencies(${TARGET_NAME} upstream_bgfx_build)
    file(MAKE_DIRECTORY "${BX_DIR}/include" "${BIMG_DIR}/include" "${BGFX_DIR}/include")
    set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION "${BGFX_OUT_DIR}/${BGFX_LIB_PREFIX}${LIB_BASE_NAME}${BGFX_CONFIG_SUFFIX}${BGFX_LIB_EXT}"
            INTERFACE_INCLUDE_DIRECTORIES "${BGFX_DIR}/include;${BIMG_DIR}/include;${BX_DIR}/include"
    )
endfunction()

add_bgfx_imported_lib(bx bx)
add_bgfx_imported_lib(bimg bimg)
add_bgfx_imported_lib(bgfx bgfx)

if(WIN32 AND NOT ANDROID)
    target_link_libraries(bgfx INTERFACE gdi32 psapi)
    target_compile_definitions(bx INTERFACE BX_CONFIG_DEBUG=0)
elseif(ANDROID)
    target_link_libraries(bgfx INTERFACE EGL GLESv2)
    target_compile_definitions(bx INTERFACE BX_CONFIG_DEBUG=0)
endif()

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
        GIT_TAG v1.93.0-docking
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
)

target_include_directories(imgui PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)