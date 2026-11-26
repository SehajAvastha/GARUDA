//
// Created by sehaj on 26-11-2026.
//

#pragma once

#include <cstdint>
#include <bgfx/bgfx.h>

namespace Garuda {
    class Window;

    class Renderer {
    public:
        Renderer() = default;
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

#if defined(GARUDA_PLATFORM_WINDOWS)
        bool Init(const Window& window, bgfx::RendererType::Enum preferredBackend = bgfx::RendererType::Direct3D11);
#else
        bool Init(const Window& window, bgfx::RendererType::Enum preferredBackend = bgfx::RendererType::Count);
#endif
        void Shutdown();

        void BeginFrame(const Window& window, uint32_t clearColorRGBA = 0x1E1E2EFF);
        void EndFrame();

        bool IsInitialized() const { return m_Initialized; }
        const char* GetBackendName() const;

    private:
        bool m_Initialized = false;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };
}