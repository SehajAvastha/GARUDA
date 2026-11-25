file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
        "Source/*.cpp"
        "Include/*.hpp"
)

if(ANDROID)
    add_library(main SHARED ${PROJECT_SOURCES})
    target_link_libraries(main PRIVATE GarudaEngine log android)
    target_compile_definitions(main PRIVATE GARUDA_PACKAGE_NAME="${GARUDA_PACKAGE_NAME}")
else()
    add_executable(${GARUDA_PROJECT_NAME} ${PROJECT_SOURCES})
    target_link_libraries(${GARUDA_PROJECT_NAME} PRIVATE GarudaEngine)
    target_compile_definitions(${GARUDA_PROJECT_NAME} PRIVATE GARUDA_PACKAGE_NAME="${GARUDA_PACKAGE_NAME}")
endif()