//
// Created by sehaj on 25-11-2026.
//

#include "Garuda/Graphics/Shader.hpp"
#include "Garuda/Core/FileSystem.hpp"
#include <SDL3/SDL_log.h>
#include <string>

namespace Garuda {

    std::string_view Shader::GetShaderBackendFolder() {
        switch (bgfx::getRendererType()) {
            case bgfx::RendererType::Direct3D11:
            case bgfx::RendererType::Direct3D12:
                return "dx11";
            case bgfx::RendererType::Vulkan:
                return "spirv";
            case bgfx::RendererType::OpenGLES:
                return "essl";
            case bgfx::RendererType::OpenGL:
                return "glsl";
            case bgfx::RendererType::Metal:
                return "metal";
            default:
                return "spirv";
        }
    }

    bgfx::ShaderHandle Shader::LoadShader(std::string_view shaderName) {
        std::string path = "Shaders/";
        path.append(GetShaderBackendFolder());
        path.push_back('/');
        path.append(shaderName);
        path.append(".bin");

        const bgfx::Memory* mem = FileSystem::ReadBgfxMemory(path);
        if (!mem) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shader binary: %s", path.c_str());
            return BGFX_INVALID_HANDLE;
        }

        bgfx::ShaderHandle handle = bgfx::createShader(mem);
        if (bgfx::isValid(handle)) {
            bgfx::setName(handle, shaderName.data(), static_cast<int32_t>(shaderName.size()));
        }
        return handle;
    }

    bgfx::ProgramHandle Shader::LoadProgram(std::string_view vertexPath, std::string_view fragmentPath) {
        bgfx::ShaderHandle vsh = LoadShader(vertexPath);
        if (!bgfx::isValid(vsh)) {
            return BGFX_INVALID_HANDLE;
        }

        bgfx::ShaderHandle fsh = LoadShader(fragmentPath);
        if (bgfx::isValid(fsh)) {
            bgfx::destroy(vsh);
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vsh, fsh, true);
    }
}
