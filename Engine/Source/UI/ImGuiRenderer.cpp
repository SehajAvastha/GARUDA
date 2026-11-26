//
// Created by sehaj on 26-11-2026.
//
#include <cstring>
#include <Garuda/UI/ImGuiRenderer.hpp>
#include <Garuda/Core/Window.hpp>

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include <bgfx/embedded_shader.h>
#include <bx/math.h>

#include <examples/common/imgui/vs_ocornut_imgui.bin.h>
#include <examples/common/imgui/fs_ocornut_imgui.bin.h>

namespace Garuda {

    bgfx::TransientVertexBuffer tvb{};
    bgfx::TransientIndexBuffer tib{};

    static const bgfx::EmbeddedShader s_ImGuiEmbeddedShaders[] = {
        BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
        BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
        BGFX_EMBEDDED_SHADER_END()
    };

    ImGuiRenderer::~ImGuiRenderer() {
        Shutdown();
    }

    bool ImGuiRenderer::Init(Window& window, bgfx::ViewId viewId, bool enableDocking) {
        if (m_Initialized) {
            return true;
        }

        m_ViewId = viewId;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        if (enableDocking) {
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

        ImGui::StyleColorsDark();

        const bgfx::RendererType::Enum type = bgfx::getRendererType();
        bool sdlInitOk = false;
        if (type == bgfx::RendererType::Direct3D11 || type == bgfx::RendererType::Direct3D12) {
            sdlInitOk = ImGui_ImplSDL3_InitForD3D(window.GetSDLWindow());
        } else {
            sdlInitOk = ImGui_ImplSDL3_InitForOther(window.GetSDLWindow());
        }

        if (!sdlInitOk) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ImGui_ImplSDL3_Init failed!");
            ImGui::DestroyContext();
            return false;
        }

        window.SetEventCallback([this](const SDL_Event& event) {
            ProcessEvent(event);
        });

        bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(s_ImGuiEmbeddedShaders, type, "vs_ocornut_imgui");
        bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(s_ImGuiEmbeddedShaders, type, "fs_ocornut_imgui");
        if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ImGuiRenderer failed to create embedded shaders!");
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

        m_Program = bgfx::createProgram(vsh, fsh, true);
        m_AttribLocationTex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);
        m_VertexLayout
            .begin()
            .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
            .end();

        m_Initialized = true;
        SDL_Log("GARUDA ImGuiRenderer initialized on view %u", static_cast<unsigned>(m_ViewId));
        return true;
    }

    void ImGuiRenderer::Shutdown() {
        if (!m_Initialized) {
            return;
        }

        for (ImTextureData* tex : ImGui::GetPlatformIO().Textures) {
            if (tex->RefCount == 1) {
                bgfx::TextureHandle handle{ static_cast<uint16_t>(tex->GetTexID()) };
                if (bgfx::isValid(handle)) {
                    bgfx::destroy(handle);
                }
                tex->SetTexID(ImTextureID_Invalid);
                tex->SetStatus(ImTextureStatus_Destroyed);
            }
        }

        if (bgfx::isValid(m_AttribLocationTex)) {
            bgfx::destroy(m_AttribLocationTex);
            m_AttribLocationTex = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(m_Program)) {
            bgfx::destroy(m_Program);
            m_Program = BGFX_INVALID_HANDLE;
        }

        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_Initialized = false;
    }

    void ImGuiRenderer::ProcessEvent(const SDL_Event& event) {
        if (m_Initialized) {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
    }

    void ImGuiRenderer::BeginFrame() {
        if (!m_Initialized) {
            return;
        }

        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiRenderer::EndFrame() {
        if (!m_Initialized) {
            return;
        }

        ImGui::Render();
        RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiRenderer::UpdateTexture(ImTextureData* tex) {
        if (tex->Status == ImTextureStatus_WantCreate) {
            bgfx::TextureHandle handle = bgfx::createTexture2D(
                static_cast<uint16_t>(tex->Width),
                static_cast<uint16_t>(tex->Height),
                false,
                1,
                bgfx::TextureFormat::RGBA8,
                0,
                nullptr
            );
            bgfx::updateTexture2D(
                handle, 0, 0, 0, 0,
                static_cast<uint16_t>(tex->Width),
                static_cast<uint16_t>(tex->Height),
                bgfx::copy(tex->GetPixels(), static_cast<uint32_t>(tex->GetSizeInBytes()))
            );
            tex->SetTexID(static_cast<ImTextureID>(handle.idx));
            tex->SetStatus(ImTextureStatus_OK);
        } else if (tex->Status == ImTextureStatus_WantUpdates) {
            bgfx::TextureHandle handle{ static_cast<uint16_t>(tex->GetTexID()) };
            if (bgfx::isValid(handle)) {
                const int ux = tex->UpdateRect.x;
                const int uy = tex->UpdateRect.y;
                const int uw = tex->UpdateRect.w;
                const int uh = tex->UpdateRect.h;
                const bgfx::Memory* mem = bgfx::alloc(static_cast<uint32_t>(uw * uh * tex->BytesPerPixel));
                const uint8_t* src = static_cast<const uint8_t*>(tex->GetPixelsAt(ux, uy));
                const int srcPitch = tex->GetPitch();
                const int dstPitch = uw * tex->BytesPerPixel;
                for (int row = 0; row < uh; ++row) {
                    std::memcpy(mem->data + row * dstPitch, src + row * srcPitch, dstPitch);
                }
                bgfx::updateTexture2D(
                    handle,
                    0,
                    0,
                    static_cast<uint16_t>(ux),
                    static_cast<uint16_t>(uy),
                    static_cast<uint16_t>(uw),
                    static_cast<uint16_t>(uh),
                    mem
                );
            }
            tex->SetStatus(ImTextureStatus_OK);
        } else if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0) {
            bgfx::TextureHandle handle{ static_cast<uint16_t>(tex->GetTexID()) };
            if (bgfx::isValid(handle)) {
                bgfx::destroy(handle);
            }
            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
        }
    }

    void ImGuiRenderer::RenderDrawData(ImDrawData* drawData) {
        if (!drawData) {
            return;
        }

        if (drawData->Textures != nullptr) {
            for (ImTextureData* tex : *drawData->Textures) {
                if (tex->Status != ImTextureStatus_OK) {
                    UpdateTexture(tex);
                }
            }
        }

        if (drawData->CmdListsCount == 0) {
            return;
        }

        const float fbWidth = drawData->DisplaySize.x * drawData->FramebufferScale.x;
        const float fbHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;
        if (fbWidth <= 0.f || fbHeight <= 0.f) {
            return;
        }

        bgfx::setViewName(m_ViewId, "ImGui");
        bgfx::setViewFrameBuffer(m_ViewId, BGFX_INVALID_HANDLE);
        bgfx::setViewMode(m_ViewId, bgfx::ViewMode::Sequential);
        bgfx::setViewRect(m_ViewId, 0, 0, static_cast<uint16_t>(fbWidth), static_cast<uint16_t>(fbHeight));

        const bgfx::Caps* caps = bgfx::getCaps();
        float ortho[16];
        const float L = drawData->DisplayPos.x;
        const float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        const float T = drawData->DisplayPos.y;
        const float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        bx::mtxOrtho(ortho, L, R, B, T, 0.f, 1000.f, 0.f, caps->homogeneousDepth);
        bgfx::setViewTransform(m_ViewId, nullptr, ortho);

        const ImVec2 clipOff = drawData->DisplayPos;
        const ImVec2 clipScale = drawData->FramebufferScale;

        for (int n = 0; n < drawData->CmdListsCount; ++n) {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            const uint32_t numVertices = static_cast<uint32_t>(cmdList->VtxBuffer.Size);
            const uint32_t numIndices = static_cast<uint32_t>(cmdList->IdxBuffer.Size);

            if (bgfx::getAvailTransientVertexBuffer(numVertices, m_VertexLayout) < numVertices ||
                bgfx::getAvailTransientIndexBuffer(numIndices, sizeof(ImDrawIdx) == 4) < numIndices) {
                break;
            }

            bgfx::TransientVertexBuffer tvb;
            bgfx::TransientIndexBuffer tib;
            bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_VertexLayout);
            bgfx::allocTransientIndexBuffer(&tib, numIndices, sizeof(ImDrawIdx) == 4);

            std::memcpy(tvb.data, cmdList->VtxBuffer.Data, numVertices * sizeof(ImDrawVert));
            std::memcpy(tib.data, cmdList->IdxBuffer.Data, numIndices * sizeof(ImDrawIdx));

            for (int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; ++cmdIdx) {
                const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdIdx];
                if (pcmd->UserCallback) {
                    pcmd->UserCallback(cmdList, pcmd);
                    continue;
                }

                if (pcmd->ElemCount == 0) {
                    continue;
                }

                const float clipX1 = (pcmd->ClipRect.x - clipOff.x) * clipScale.x;
                const float clipY1 = (pcmd->ClipRect.y - clipOff.y) * clipScale.y;
                const float clipX2 = (pcmd->ClipRect.z - clipOff.x) * clipScale.x;
                const float clipY2 = (pcmd->ClipRect.w - clipOff.y) * clipScale.y;

                if (clipX2 <= clipX1 || clipY2 <= clipY1) {
                    continue;
                }

                const uint16_t xx = static_cast<uint16_t>(bx::max(clipX1, 0.f));
                const uint16_t yy = static_cast<uint16_t>(bx::max(clipY1, 0.f));
                bgfx::setScissor(
                    xx,
                    yy,
                    static_cast<uint16_t>(bx::min(clipX2, 65535.0f) - xx),
                    static_cast<uint16_t>(bx::min(clipY2, 65535.0f) - yy)
                );

                const ImTextureID cmdTexId = pcmd->GetTexID();
                if (cmdTexId == ImTextureID_Invalid) {
                    continue;
                }

                bgfx::TextureHandle texHandle{ static_cast<uint16_t>(cmdTexId) };
                if (!bgfx::isValid(texHandle)) {
                    continue;
                }
                bgfx::setState(
                BGFX_STATE_WRITE_RGB |
                BGFX_STATE_WRITE_A |
                BGFX_STATE_MSAA |
                BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                );
                bgfx::setTexture(0, m_AttribLocationTex, texHandle);
                bgfx::setVertexBuffer(0, &tvb, pcmd->VtxOffset, numVertices);
                bgfx::setIndexBuffer(&tib, pcmd->IdxOffset, pcmd->ElemCount);
                bgfx::submit(m_ViewId, m_Program);
            }
        }
    }
}
