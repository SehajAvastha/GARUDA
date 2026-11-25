file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        "Source/*.cpp"
        "Include/*.hpp"
)

add_library(GarudaEngine STATIC ${ENGINE_SOURCES})

target_include_directories(GarudaEngine PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

target_compile_definitions(GarudaEngine PUBLIC
        $<$<PLATFORM_ID:ANDROID>:GARUDA_PLATFORM_ANDROID>
        $<$<PLATFORM_ID:WINDOWS>:GARUDA_PLATFORM_WINDOWS>
)

target_link_libraries(GarudaEngine PUBLIC
        SDL3::SDL3
        bx
        bgfx
        bimg
        Jolt
        box2d
        glm::glm
        stb_headers
        imgui
)