file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        "Source/*.cpp"
        "Include/*.hpp"
)

add_library(GarudaEngine STATIC ${ENGINE_SOURCES})

add_dependencies(GarudaEngine upstream_bgfx_build)

target_include_directories(GarudaEngine PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
        "${BGFX_DIR}/include"
        "${BIMG_DIR}/include"
        "${BX_DIR}/include"
)

target_compile_definitions(GarudaEngine PUBLIC
        $<$<PLATFORM_ID:Android>:GARUDA_PLATFORM_ANDROID>
        $<$<PLATFORM_ID:Windows>:GARUDA_PLATFORM_WINDOWS>
)

target_link_libraries(GarudaEngine PUBLIC
        SDL3::SDL3
        bgfx
        bimg
        bx
        Jolt
        box2d
        glm::glm
        stb_headers
        imgui
)