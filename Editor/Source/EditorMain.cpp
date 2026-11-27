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

#include <Garuda/Core/FileSystem.hpp>
#include <Garuda/Graphics/Camera.hpp>
#include <Garuda/Graphics/Model.hpp>
#include <Garuda/Graphics/Shader.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    // Editor Camera
    Garuda::Camera editorCamera;
    editorCamera.SetPerspective(60.f, 0.1f, 100.f);
    editorCamera.SetPosition({ -3.5f, 1.5f, 0.f });
    editorCamera.LookAt({ 0.f, 0.f, 0.f });

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
            editorCamera.SetViewportSize(
                static_cast<float>(viewportFbo.GetWidth()),
                static_cast<float>(viewportFbo.GetHeight())
            );
            editorCamera.ApplyToView(1);

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
        ImGui::Separator();
        ImGui::Text("Editor Camera");
        const char* projTypes[] = { "Perspective", "Orthographic" };
        int currentProj = static_cast<int>(editorCamera.GetProjectionType());
        if (ImGui::Combo("Projection", &currentProj, projTypes, 2)) {
            editorCamera.SetProjectionType(static_cast<Garuda::ProjectionType>(currentProj));
        }

        if (editorCamera.GetProjectionType() == Garuda::ProjectionType::Perspective) {
            ImGui::SliderFloat("FOV", &editorCamera.GetPerspectiveFOV(), 15.0f, 120.0f);
            ImGui::DragFloat("Near", &editorCamera.GetPerspectiveNear(), 0.01f, 0.001f, 10.0f);
            ImGui::DragFloat("Far", &editorCamera.GetPerspectiveFar(), 1.0f, 10.0f, 5000.0f);
        } else {
            ImGui::DragFloat("Ortho Size", &editorCamera.GetOrthoSize(), 0.1f, 0.5f, 100.0f);
            ImGui::DragFloat("Ortho Near", &editorCamera.GetOrthoNear(), 0.5f, -1000.0f, 0.0f);
            ImGui::DragFloat("Ortho Far", &editorCamera.GetOrthoFar(), 0.5f, 1.0f, 1000.0f);
        }

        ImGui::DragFloat3("Position", &editorCamera.GetPosition().x, 0.05f);
        if (ImGui::DragFloat3("Rotation", &editorCamera.GetRotation().x, 0.5f)) {
            editorCamera.SetRotation(editorCamera.GetRotation());
        }
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
