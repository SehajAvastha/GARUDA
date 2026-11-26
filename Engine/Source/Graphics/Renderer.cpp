//
// Created by sehaj on 26-11-2026.
//

#include "Garuda/Graphics/Renderer.hpp"
#include "Garuda/Core/Window.hpp"
#include <SDL3/SDL_log.h>

namespace Garuda {
    Renderer::~Renderer() {
        Shutdown();
    }

    bool Renderer::Init(const Window &window, bgfx::RendererType::Enum preferredBackend) {
        if (m_Initialized) {
            return true;
        }

        void* nativeHandle = window.GetNativeHandle();
        if (!nativeHandle) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer::Init() failed: native window handle is null");
            return false;
        }

        m_Width = static_cast<uint32_t>(window.GetWidth());
        m_Height = static_cast<uint32_t>(window.GetHeight());

        bgfx::renderFrame();

        bgfx::Init init{};
        init.type = preferredBackend;
        init.swapChain.nwh = nativeHandle;
        init.swapChain.width = m_Width;
        init.swapChain.height = m_Height;
        init.reset = BGFX_RESET_VSYNC;

        if (!bgfx::init(init)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer::Init() failed: bgfx::init returned false");
            return false;
        }

        m_Initialized = true;
        SDL_Log("GARUDA Renderer initialized: %s (%ux%u)", GetBackendName(), m_Width, m_Height);
        return true;
    }

    void Renderer::Shutdown() {
        if (m_Initialized) {
            bgfx::shutdown();
            m_Initialized = false;
        }
    }

    void Renderer::BeginFrame(const Window &window, uint32_t clearColorRGBA) {
        if (!m_Initialized || window.IsSuspended()) {
            return;
        }

        if (window.WasSurfaceRecreated()) {
            bgfx::SwapChain sc {};
            sc.nwh = window.GetNativeHandle();
            m_Width = static_cast<uint32_t>(window.GetWidth());
            m_Height = static_cast<uint32_t>(window.GetHeight());
            bgfx::reset(BGFX_RESET_VSYNC, &sc);
        } else if (window.WasResized()) {
            const uint32_t nWidth = static_cast<uint32_t>(window.GetWidth());
            const uint32_t nHeight = static_cast<uint32_t>(window.GetHeight());
            if (nWidth > 0 && nHeight > 0 && (nWidth != m_Width || nHeight != m_Height)) {
                m_Width = nWidth;
                m_Height = nHeight;

                bgfx::SwapChain sc{};
                sc.nwh = window.GetNativeHandle();
                sc.width = m_Width;
                sc.height = m_Height;
                bgfx::reset(BGFX_RESET_VSYNC, &sc);
            }
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColorRGBA, 1.f, 0);
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_Width), static_cast<uint16_t>(m_Height));
        bgfx::touch(0);
    }

    void Renderer::EndFrame() {
        if (!m_Initialized) {
            return;
        }
        bgfx::frame();
    }

    const char *Renderer::GetBackendName() const {
        if (!m_Initialized) {
            return "Uninitialized";
        }
        return bgfx::getRendererName(bgfx::getRendererType());
    }
}
