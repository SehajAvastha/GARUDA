//
// Created by sehaj on 25-11-2026.
//

#include <Garuda/Core.hpp>
#include <Garuda/Core/FileSystem.hpp>
#include <SDL3/SDL.h>

namespace Garuda {
    void Initialize() {

#if defined(GARUDA_PLATFORM_ANDROID)
        FileSystem::SetRootPath("");
#else
        const char* basePath = SDL_GetBasePath();
        if (basePath) {
            FileSystem::SetRootPath(std::string(basePath) + "Assets/");
        } else {
            FileSystem::SetRootPath("Assets/");
        }
#endif

        SDL_Log("GARUDA Engine Initialized!");
    }

    void Shutdown() {
        SDL_Log("GARUDA Engine Shutdown.");
    }
}