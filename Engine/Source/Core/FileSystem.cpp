//
// Created by sehaj on 25-11-2026.
//

#include "Garuda/Core/FileSystem.hpp"
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <cstring>

namespace Garuda {
    static std::string s_RootPath;

    void FileSystem::SetRootPath(std::string_view path) {
        s_RootPath = path;
        if (!s_RootPath.empty() && s_RootPath.back() != '/' && s_RootPath.back() != '\\') {
            s_RootPath.push_back('/');
        }
    }

    const std::string& FileSystem::GetRootPath() {
        return s_RootPath;
    }

    std::string FileSystem::ResolvedPath(std::string_view path) {
        if (s_RootPath.empty()) {
            return std::string(path);
        }
        std::string fullPath = s_RootPath;
        if (!path.empty() && (path.front() == '/' || path.front() == '\\')) {
            fullPath.append(path.substr(1));
        } else {
            fullPath.append(path);
        }
        return fullPath;
    }

    std::vector<uint8_t> FileSystem::ReadBinary(std::string_view path) {
        const std::string filePath = ResolvedPath(path);
        SDL_IOStream* stream = SDL_IOFromFile(filePath.c_str(), "rb");
        if (!stream) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FileSystem::ReadBinary() failed for '%s' : %s", filePath.c_str(), SDL_GetError());
            return {};
        }

        const Sint64 size = SDL_GetIOSize(stream);
        if (size <= 0) {
            SDL_CloseIO(stream);
            return {};
        }

        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        const size_t bytesRead = SDL_ReadIO(stream, buffer.data(), buffer.size());
        SDL_CloseIO(stream);

        if (bytesRead != buffer.size()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FileSystem::ReadBinary() partial read for '%s'", filePath.c_str());
            buffer.resize(bytesRead);
        }
        return buffer;
    }

    std::string FileSystem::ReadText(std::string_view path) {
        const std::vector<uint8_t> bytes = ReadBinary(path);
        if (bytes.empty()) {
            return {};
        }

        return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }

    const bgfx::Memory* FileSystem::ReadBgfxMemory(std::string_view path) {
        const std::string filePath = ResolvedPath(path);
        SDL_IOStream* stream = SDL_IOFromFile(filePath.c_str(), "rb");
        if (!stream) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FileSystem::ReadBgfxMemory() failed for '%s' : %s", filePath.c_str(), SDL_GetError());
            return nullptr;
        }

        const Sint64 size = SDL_GetIOSize(stream);
        if (size <= 0) {
            SDL_CloseIO(stream);
            return nullptr;
        }

        const bgfx::Memory* mem = bgfx::alloc(static_cast<uint32_t>(size) + 1);
        const size_t bytesRead = SDL_ReadIO(stream, mem->data, static_cast<size_t>(size));
        SDL_CloseIO(stream);

        if (bytesRead != static_cast<size_t>(size)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FileSystem::ReadMemory() partial read for '%s'", filePath.c_str());
            return nullptr;
        }

        mem->data[size] = '\0';
        return mem;
    }
}