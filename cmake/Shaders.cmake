function(garuda_compile_shaders TARGET_NAME)
    if(ANDROID)
        return()
    endif()

    set(SHADERC_EXE "${BGFX_OUT_DIR}/shadercRelease.exe")
    set(BGFX_SHADER_INC "${BGFX_DIR}/src")
    set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/Assets/Shaders")
    set(VARYING_DEF "${SHADER_SRC_DIR}/varying.def.sc")

    set(VERTEX_SHADERS vs_basic)
    set(FRAGMENT_SHADERS fs_basic)

    # Compile Direct3D 11 (Windows default) and SPIR-V (Vulkan)
    set(BACKENDS dx11 spirv)
    set(ALL_COMPILED_BINS "")

    foreach(BACKEND ${BACKENDS})
        set(OUT_DIR "${CMAKE_SOURCE_DIR}/Assets/Shaders/${BACKEND}")
        file(MAKE_DIRECTORY "${OUT_DIR}")

        if(BACKEND STREQUAL "dx11")
            set(PLATFORM_ARG windows)
            set(VS_PROFILE s_5_0)
            set(FS_PROFILE s_5_0)
        elseif(BACKEND STREQUAL "spirv")
            set(PLATFORM_ARG linux)
            set(VS_PROFILE spirv)
            set(FS_PROFILE spirv)
        endif()

        foreach(VS ${VERTEX_SHADERS})
            set(SRC "${SHADER_SRC_DIR}/${VS}.sc")
            set(BIN "${OUT_DIR}/${VS}.bin")
            add_custom_command(
                    OUTPUT "${BIN}"
                    COMMAND "${SHADERC_EXE}" -f "${SRC}" -o "${BIN}" --type v --platform ${PLATFORM_ARG} -p ${VS_PROFILE} -i "${BGFX_SHADER_INC}" --varyingdef "${VARYING_DEF}"
                    DEPENDS "${SRC}" "${VARYING_DEF}" upstream_bgfx_build
                    COMMENT "Compiling vertex shader ${VS}.sc (${BACKEND})..."
                    VERBATIM
            )
            list(APPEND ALL_COMPILED_BINS "${BIN}")
        endforeach()

        foreach(FS ${FRAGMENT_SHADERS})
            set(SRC "${SHADER_SRC_DIR}/${FS}.sc")
            set(BIN "${OUT_DIR}/${FS}.bin")
            add_custom_command(
                    OUTPUT "${BIN}"
                    COMMAND "${SHADERC_EXE}" -f "${SRC}" -o "${BIN}" --type f --platform ${PLATFORM_ARG} -p ${FS_PROFILE} -i "${BGFX_SHADER_INC}" --varyingdef "${VARYING_DEF}"
                    DEPENDS "${SRC}" "${VARYING_DEF}" upstream_bgfx_build
                    COMMENT "Compiling fragment shader ${FS}.sc (${BACKEND})..."
                    VERBATIM
            )
            list(APPEND ALL_COMPILED_BINS "${BIN}")
        endforeach()
    endforeach()

    add_custom_target(${TARGET_NAME}_shaders DEPENDS ${ALL_COMPILED_BINS})
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
endfunction()
