//
// Created by sehaj on 26-11-2026.
//

#include <Garuda/Graphics/FrameBuffer.hpp>
#include <SDL3/SDL_log.h>

namespace Garuda {
    FrameBuffer::~FrameBuffer() {
        Destroy();
    }

    bool FrameBuffer::Create(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return false;
        }

        Destroy();

        m_Width = width;
        m_Height = height;

        const uint64_t textureFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

        m_ColorTexture = bgfx::createTexture2D(
            static_cast<uint16_t>(m_Width),
            static_cast<uint16_t>(m_Height),
            false,
            1,
            bgfx::TextureFormat::RGBA8,
            textureFlags
        );

        m_DepthTexture = bgfx::createTexture2D(
            static_cast<uint16_t>(m_Width),
            static_cast<uint16_t>(m_Height),
            false,
            1,
            bgfx::TextureFormat::D24S8,
            textureFlags
        );

        if (!bgfx::isValid(m_ColorTexture) || !bgfx::isValid(m_DepthTexture)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Framebuffer::Create() failed to create textures (%ux%u)", m_Width, m_Height);
            Destroy();
            return false;
        }

        bgfx::TextureHandle attachments[] = { m_ColorTexture, m_DepthTexture };
        m_FboHandle = bgfx::createFrameBuffer(2, attachments, true);

        if (!bgfx::isValid(m_FboHandle)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Framebuffer::Create() failed to create FrameBufferHandle");
            Destroy();
            return false;
        }

        return true;
    }

    void FrameBuffer::Destroy() {
        if (bgfx::isValid(m_FboHandle)) {
            bgfx::destroy(m_FboHandle);
            m_FboHandle = BGFX_INVALID_HANDLE;
            m_ColorTexture = BGFX_INVALID_HANDLE;
            m_DepthTexture = BGFX_INVALID_HANDLE;
        } else {
            if (bgfx::isValid(m_ColorTexture)) {
                bgfx::destroy(m_ColorTexture);
                m_ColorTexture = BGFX_INVALID_HANDLE;
            }
            if (bgfx::isValid(m_DepthTexture)) {
                bgfx::destroy(m_DepthTexture);
                m_DepthTexture = BGFX_INVALID_HANDLE;
            }
        }

        m_Width = 0;
        m_Height = 0;
    }

    void FrameBuffer::Resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }
        if (width == m_Width && height == m_Height && IsValid()) {
            return;
        }
        Create(width, height);
    }

    void FrameBuffer::BindAndClear(bgfx::ViewId viewId, uint32_t clearColorRGBA) const {
        if (!IsValid()) {
            return;
        }

        bgfx::setViewFrameBuffer(viewId, m_FboHandle);
        bgfx::setViewRect(viewId, 0, 0, static_cast<uint16_t>(m_Width), static_cast<uint16_t>(m_Height));
        bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColorRGBA, 1.0f, 0);
        bgfx::touch(viewId);
    }
}
