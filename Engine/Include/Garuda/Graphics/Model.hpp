//
// Created by sehaj on 26-11-2026.
//

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace Garuda {

    struct MeshVertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
        uint32_t abgr;
    };

    struct SubMesh {
        bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
        uint32_t indexCount = 0;
        glm::vec4 baseColor = { 1.f, 1.f, 1.f, 1.f };
        std::string name;

        void Destroy();
    };

    class Model {
    public:
        Model() = default;
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&& other) noexcept;
        Model& operator=(Model&& other) noexcept;

        bool LoadFromFile(std::string_view assetRelativePath);
        void Destroy();

        void Submit(bgfx::ViewId viewId, bgfx::ProgramHandle program, const glm::mat4& transform, uint64_t state = 0) const;

        bool IsLoaded() const { return !m_SubMeshes.empty(); }
        const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
        const glm::vec3& GetBoundsMin() const { return m_BoundsMin; }
        const glm::vec3& GetBoundsMax() const { return m_BoundsMax; }

    private:
        std::vector<SubMesh> m_SubMeshes;
        glm::vec3 m_BoundsMin{ 0.f };
        glm::vec3 m_BoundsMax{ 0.f };
    };
}