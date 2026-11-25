//
// Created by sehaj on 25-11-2026.
//

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Garuda/Core.hpp"
#include "Garuda/Core/Window.hpp"

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);
    Garuda::Initialize();

    Garuda::Window window;
    {
        if (!window.Create({"GARUDA Window Test", 1280, 720, true })) {
            return 1;
        }

        while (window.PollEvents()) {
            //
        }
    }

    Garuda::Shutdown();
    SDL_Quit();

    return 0;
}