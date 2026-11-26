//
// Created by sehaj on 25-11-2026.
//

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <Garuda/Core.hpp>
#include <Garuda/Core/Window.hpp>
#include <Garuda/Graphics/Renderer.hpp>
#include <Garuda/UI/ImGuiRenderer.hpp>

int main(int argc, char* argv[]) {
    Garuda::Initialize();

    Garuda::Window window;

    if (!window.Create({"GARUDA Window Test", 1280, 720, true })) {
        return 1;
    }

    Garuda::Renderer renderer;
    if (!renderer.Init(window)) {
        return 1;
    }

    Garuda::ImGuiRenderer imgui;
    if (!imgui.Init(window)) {
        return 1;
    }

    while (window.PollEvents()) {
        if (window.IsSuspended()) {
            continue;
        }

        renderer.BeginFrame(window, 0xFF0000FF);
        imgui.BeginFrame();

        ImGui::Begin("GARUDA Project Diagnostics");
        ImGui::Text("Graphics Backend: %s", renderer.GetBackendName());
        ImGui::Text("Resolution: %d x %d", window.GetWidth(), window.GetHeight());
        ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
        ImGui::End();

        imgui.EndFrame();
        renderer.EndFrame();
    }

    imgui.Shutdown();
    renderer.Shutdown();
    window.Destroy();
    Garuda::Shutdown();

    return 0;
}