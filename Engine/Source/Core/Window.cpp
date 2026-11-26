//
// Created by sehaj on 25-11-2026.
//

#include "Garuda/Core/Window.hpp"
#include <SDL3/SDL.h>

namespace Garuda {
    Window::~Window() {
        Destroy();
    }

    bool Window::Create(const WindowConfig& config) {
        if (!SDL_WasInit(SDL_INIT_VIDEO)) {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
                return false;
            }
        }

        SDL_WindowFlags flags = 0;
#if defined(GARUDA_PLATFORM_ANDROID)
        flags |= SDL_WINDOW_FULLSCREEN;
#else
        if (config.resizeable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }
#endif

        m_Window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
        if (!m_Window) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
            return false;
        }

        SDL_GetWindowSize(m_Window, &m_Width, &m_Height);
        m_ShouldClose = false;
        m_Resized = false;
        return true;
    }

    void Window::Destroy() {
        if (m_Window) {
            SDL_DestroyWindow(m_Window);
            m_Window = nullptr;
        }
        if (SDL_WasInit(SDL_INIT_VIDEO)) {
            SDL_Quit();
        }
    }

    bool Window::PollEvents() {
        m_Resized = false;
        m_SurfaceRecreated = false;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_TERMINATING:
                    m_ShouldClose = true;
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    SDL_GetWindowSizeInPixels(m_Window, &m_Width, &m_Height);
                    m_Resized = true;
                    break;

                case SDL_EVENT_WILL_ENTER_BACKGROUND:
                    m_IsSuspended = true;
                    break;

                case SDL_EVENT_DID_ENTER_FOREGROUND:
                    m_IsSuspended = false;
                    m_SurfaceRecreated = true;
                    SDL_GetWindowSizeInPixels(m_Window, &m_Width, &m_Height);
                    break;

                default:
                    break;
            }
        }

        return !m_ShouldClose;
    }

    void* Window::GetNativeHandle() const {
        if (!m_Window) {
            return nullptr;
        }

        SDL_PropertiesID props = SDL_GetWindowProperties(m_Window);
#if defined(GARUDA_PLATFORM_WINDOWS) || defined(_WIN32) || defined(_WIN64)
        return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(GARUDA_PLATFORM_ANDROID) || defined(__ANDROID__)
        return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
#else
        return nullptr;
#endif
    }
}
