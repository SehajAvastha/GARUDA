file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
        "Source/*.cpp"
        "Include/*.hpp"
)

add_executable(GarudaEditor ${EDITOR_SOURCES})

target_compile_definitions(GarudaEditor PRIVATE
        GARUDA_SOURCE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets/"
)

target_include_directories(GarudaEditor PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

target_link_libraries(GarudaEditor PRIVATE GarudaEngine)

garuda_compile_shaders(GarudaEditor)