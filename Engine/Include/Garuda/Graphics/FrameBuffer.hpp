//
// Created by sehaj on 26-11-2026.
//

#pragma once

#include <cstdint>
#include <bgfx/bgfx.h>

namespace Garuda {

    class FrameBuffer {
    public:
        FrameBuffer() = default;
        ~FrameBuffer();

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        bool Create(uint32_t width, uint32_t height);
        void Destroy();
        void Resize(uint32_t width, uint32_t height);

        void BindAndClear(bgfx::ViewId viewId, uint32_t clearColorRGBA = 0x1E1E2EFF) const;

        bool IsValid() const { return bgfx::isValid(m_FboHandle); }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        bgfx::TextureHandle GetColorTexture() const { return m_ColorTexture; }
        bgfx::FrameBufferHandle GetHandle() const { return m_FboHandle; }

    private:
        bgfx::FrameBufferHandle m_FboHandle = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle m_ColorTexture = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle m_DepthTexture = BGFX_INVALID_HANDLE;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };
}