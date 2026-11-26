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

#include "Garuda/Core/FileSystem.hpp"
#include "Garuda/Graphics/Model.hpp"
#include "Garuda/Graphics/Shader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

int main(int argc, char* argv[]) {
    Garuda::Initialize();

#ifdef GARUDA_SOURCE_ASSETS_DIR
    Garuda::FileSystem::SetRootPath(GARUDA_SOURCE_ASSETS_DIR);
#endif

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
    viewportFbo.Create(1000, 720);

    ImVec2 viewportSize = { 1000.0f, 720.0f };
    float viewportClearColor[4] = { 0.784313725f, 0.784313725f, 0.784313725f, 1.0f };
    bool firstFrame = true;

    // Load Shader Program
    bgfx::ProgramHandle basicProgram = Garuda::Shader::LoadProgram("vs_basic", "fs_basic");

    // Custom Cube Import
    Garuda::Model importedModel;
    importedModel.LoadFromFile("Models/Cube.gltf");
    float rotationSpeed = 1.f;
    float rotationAngle = 0.f;

    while (window.PollEvents()) {
        if (window.IsSuspended()) {
            continue;
        }

        // custom modification
        rotationAngle += ImGui::GetIO().DeltaTime * rotationSpeed;

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

        // Submit imported 3D model to View 1
        if (bgfx::isValid(basicProgram) && importedModel.IsLoaded() && viewportFbo.IsValid()) {
            const float aspect = static_cast<float>(viewportFbo.GetWidth()) / static_cast<float>(viewportFbo.GetHeight());
            const glm::mat4 view = glm::lookAtLH(
                glm::vec3(0.f, 1.5f, -3.5f),
                glm::vec3(0.f, 0.f, 0.f),
                glm::vec3(0.f, 1.f, 0.f)
            );
            glm::mat4 proj = glm::perspectiveLH_ZO(glm::radians(60.f), aspect, 0.1f, 100.f);
            if (bgfx::getCaps()->originBottomLeft) {
                proj = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f)) * proj;
            }
            bgfx::setViewTransform(1, glm::value_ptr(view), glm::value_ptr(proj));

            glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.f), rotationAngle, glm::vec3(0.3f, 1.f, 0.f));
            importedModel.Submit(1, basicProgram, modelMatrix);
        }

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

    importedModel.Destroy();
    if (bgfx::isValid(basicProgram)) {
        bgfx::destroy(basicProgram);
    }

    viewportFbo.Destroy();
    imgui.Shutdown();
    renderer.Shutdown();
    window.Destroy();
    Garuda::Shutdown();

    return 0;
}
