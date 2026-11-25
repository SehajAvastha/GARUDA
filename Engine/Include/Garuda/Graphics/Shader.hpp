//
// Created by sehaj on 25-11-2026.
//

#pragma once

#include <string_view>
#include <bgfx/bgfx.h>

namespace Garuda {

    class Shader {
    public:

        static bgfx::ShaderHandle LoadShader(std::string_view shaderName);

        static bgfx::ProgramHandle LoadProgram(std::string_view vertexPath, std::string_view fragmentPath);

        static std::string_view GetShaderBackendFolder();
    };
}