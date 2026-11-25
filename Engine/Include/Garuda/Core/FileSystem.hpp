//
// Created by sehaj on 25-11-2026.
//

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <bgfx/bgfx.h>

namespace Garuda {
    class FileSystem {
    public:

        static void SetRootPath(std::string_view rootPath);
        static const std::string& GetRootPath();

        static std::string ResolvedPath(std::string_view path);

        static std::vector<uint8_t> ReadBinary(std::string_view path);
        static std::string ReadText(std::string_view path);

        static const bgfx::Memory* ReadBgfxMemory(std::string_view path);
    };
}