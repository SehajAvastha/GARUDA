//
// Created by sehaj on 26-11-2026.
//

#pragma once

#include <cstdint>
#include <bgfx/bgfx.h>
#include <imgui.h>

union SDL_Event;

namespace Garuda {

    class Window;

    class ImGuiRenderer {
    public:
        ImGuiRenderer() = default;
        ~ImGuiRenderer();

        ImGuiRenderer(const ImGuiRenderer&) = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

        bool Init(Window& window, bgfx::ViewId viewId = 255, bool enableDocking = true);
        void Shutdown();

        void ProcessEvent(const SDL_Event& event);
        void BeginFrame();
        void EndFrame();

        bool IsInitialized() const { return m_Initialized; }

    private:
        void UpdateTexture(ImTextureData* tex);
        void RenderDrawData(ImDrawData* drawData);

        bool m_Initialized = false;
        bgfx::ViewId m_ViewId = 255;
        bgfx::VertexLayout m_VertexLayout{};
        bgfx::ProgramHandle m_Program = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_AttribLocationTex = BGFX_INVALID_HANDLE;
    };

}