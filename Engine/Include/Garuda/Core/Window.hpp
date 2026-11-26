//
// Created by sehaj on 25-11-2026.
//

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

#include "SDL3/SDL_events.h"

struct SDL_Window;

namespace Garuda {

    struct WindowConfig {
        std::string title = "GARUDA Engine";
        int width = 1280;
        int height = 720;
        bool resizeable = false;
    };

    class Window {
    public:
        using EventCallback = std::function<void(const SDL_Event&)>;

        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool Create(const WindowConfig& config = {});
        void Destroy();

        bool PollEvents();

        void SetEventCallback(EventCallback callback) { m_EventCallback = std::move(callback); }

        bool IsOpen() const { return m_Window != nullptr && !m_ShouldClose; }
        bool IsSuspended() const { return m_IsSuspended; }
        bool WasResized() const { return m_Resized; }
        bool WasSurfaceRecreated() const { return m_SurfaceRecreated; }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        void* GetNativeHandle() const;
        SDL_Window* GetSDLWindow() const { return m_Window; }

    private:
        SDL_Window* m_Window = nullptr;
        int m_Width = 0;
        int m_Height = 0;
        bool m_ShouldClose = false;
        bool m_Resized = false;
        bool m_IsSuspended = false;
        bool m_SurfaceRecreated = false;
        EventCallback m_EventCallback;
    };
}