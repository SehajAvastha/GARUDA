//
// Created by sehaj on 25-11-2026.
//

#include "Garuda/Core.hpp"
#include <SDL3/SDL.h>

namespace Garuda {
    void Initialize() {
        SDL_LOG("GARUDA Engine Initialized!");
    }

    void Shutdown() {
        SDL_LOG("GARUDA Engien Shutdown.");
    }
}