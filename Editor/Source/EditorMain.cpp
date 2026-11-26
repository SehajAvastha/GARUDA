//
// Created by sehaj on 25-11-2026.
//

#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "Garuda/Core.hpp"
#include <Garuda/Core/Window.hpp>
#include <Garuda/Graphics/Renderer.hpp>
#include <Garuda/Graphics/Framebuffer.hpp>
#include <Garuda/UI/ImGuiRenderer.hpp>

int main(int argc, char* argv[]) {
    Garuda::Initialize();

    Garuda::Window window;
    if (!window.Create({"GARUDA Editor", 1280, 720, true})) {
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

    Garuda::FrameBuffer viewportFbo;
    viewportFbo.Create(960, 720);

    ImVec2 viewportSize = { 960.0f, 720.0f };
    float viewportClearColor[4] = { 0.14f, 0.22f, 0.36f, 1.0f };
    bool firstFrame = true;

    while (window.PollEvents()) {
        if (window.IsSuspended()) {
            continue;
        }

        // Resize offscreen Viewport FBO if panel size changed
        const uint32_t targetW = static_cast<uint32_t>(viewportSize.x);
        const uint32_t targetH = static_cast<uint32_t>(viewportSize.y);
        if (targetW > 0 && targetH > 0) {
            viewportFbo.Resize(targetW, targetH);
        }

        // View 0: Main Window Backbuffer
        renderer.BeginFrame(window, 0x16161EFF);

        // View 1: Offscreen Viewport Framebuffer
        const uint32_t clearRGBA =
            (static_cast<uint32_t>(viewportClearColor[0] * 255.0f) << 24) |
            (static_cast<uint32_t>(viewportClearColor[1] * 255.0f) << 16) |
            (static_cast<uint32_t>(viewportClearColor[2] * 255.0f) << 8)  |
            (static_cast<uint32_t>(viewportClearColor[3] * 255.0f));
        viewportFbo.BindAndClear(1, clearRGBA);

        // View 255: Dear ImGui Editor UI
        imgui.BeginFrame();

        const ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        // Build default split layout once on startup
        if (firstFrame) {
            firstFrame = false;
            ImGui::DockBuilderRemoveNode(dockspaceId);
            ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

            ImGuiID dockLeft = 0;
            ImGuiID dockRight = 0;
            ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.25f, &dockLeft, &dockRight);

            ImGui::DockBuilderDockWindow("GARUDA Engine Diagnostics", dockLeft);
            ImGui::DockBuilderDockWindow("Viewport", dockRight);
            ImGui::DockBuilderFinish(dockspaceId);
        }

        // 1. Diagnostics & Viewport Settings Panel
        ImGui::Begin("GARUDA Engine Diagnostics");
        ImGui::Text("Graphics Backend: %s", renderer.GetBackendName());
        ImGui::Text("Window: %d x %d", window.GetWidth(), window.GetHeight());
        ImGui::Text("Viewport: %u x %u", viewportFbo.GetWidth(), viewportFbo.GetHeight());
        ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
        ImGui::Separator();
        ImGui::ColorEdit4("Viewport Clear Color", viewportClearColor);
        ImGui::End();

        // 2. Live Offscreen Viewport Panel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        viewportSize = ImGui::GetContentRegionAvail();
        if (viewportFbo.IsValid() && viewportSize.x > 0.0f && viewportSize.y > 0.0f) {
            const ImTextureID texId = static_cast<ImTextureID>(viewportFbo.GetColorTexture().idx);
            ImGui::Image(texId, viewportSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();

        imgui.EndFrame();
        renderer.EndFrame();
    }

    viewportFbo.Destroy();
    imgui.Shutdown();
    renderer.Shutdown();
    window.Destroy();
    Garuda::Shutdown();

    return 0;
}