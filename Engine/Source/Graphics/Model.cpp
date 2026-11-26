//
// Created by sehaj on 26-11-2026.
//

#include <Garuda/Graphics/Model.hpp>
#include <Garuda/Core/FileSystem.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL_log.h>
#include <algorithm>
#include <limits>

namespace Garuda {

    static bgfx::VertexLayout s_MeshVertexLayout;
    static bool s_LayoutInitialized = false;

    static const bgfx::VertexLayout& GetMeshVertexLayout() {
        if (!s_LayoutInitialized) {
            s_MeshVertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
            s_LayoutInitialized = true;
        }
        return s_MeshVertexLayout;
    }

    static uint32_t PackColorAGBR(const glm::vec4& c) {
        const auto r = static_cast<uint32_t>(std::clamp(c.r, 0.f, 1.f) * 255.f);
        const auto g = static_cast<uint32_t>(std::clamp(c.g, 0.f, 1.f) * 255.f);
        const auto b = static_cast<uint32_t>(std::clamp(c.b, 0.f, 1.f) * 255.f);
        const auto a = static_cast<uint32_t>(std::clamp(c.a, 0.f, 1.f) * 255.f);

        return (a << 24) | (b << 16) | (g << 8) | r;
    }

    void SubMesh::Destroy() {
        if (bgfx::isValid(vbh)) {
            bgfx::destroy(vbh);
            vbh = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(ibh)) {
            bgfx::destroy(ibh);
            ibh = BGFX_INVALID_HANDLE;
        }
        indexCount = 0;
    }

    Model::~Model() {
        Destroy();
    }

    Model::Model(Model &&other) noexcept : m_SubMeshes(std::move(other.m_SubMeshes)), m_BoundsMin(other.m_BoundsMin), m_BoundsMax(other.m_BoundsMax) {}

    Model &Model::operator=(Model &&other) noexcept {
        if (this != &other) {
            Destroy();
            m_SubMeshes = std::move(other.m_SubMeshes);
            m_BoundsMin = other.m_BoundsMin;
            m_BoundsMax = other.m_BoundsMax;
        }
        return *this;
    }

    void Model::Destroy() {
        for (auto& sub : m_SubMeshes) {
            sub.Destroy();
        }
        m_SubMeshes.clear();
    }

    bool Model::LoadFromFile(std::string_view assetRelativePath) {
        Destroy();

        std::vector<uint8_t> fileBytes = FileSystem::ReadBinary(assetRelativePath);
        if (fileBytes.empty()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Model::LoadFromFile failed to read: %. *s",
                static_cast<int>(assetRelativePath.size()), assetRelativePath.data());
            return false;
        }

        std::string extHint;
        const size_t dotPos = assetRelativePath.find_last_of('.');
        if (dotPos != std::string::npos && dotPos + 1 < assetRelativePath.size()) {
            extHint = std::string(assetRelativePath.substr(dotPos + 1));
        }

        Assimp::Importer importer;
        const unsigned int postProcessFlags =
                aiProcess_Triangulate |
                    aiProcess_GenSmoothNormals |
                        aiProcess_CalcTangentSpace |
                            aiProcess_JoinIdenticalVertices |
                                aiProcess_ConvertToLeftHanded;

        const aiScene* scene = importer.ReadFileFromMemory(
            fileBytes.data(),
            fileBytes.size(),
            postProcessFlags,
            extHint.c_str()
        );

        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Assimp error loading: %%. %s: %s",
                static_cast<int>(assetRelativePath.size()), assetRelativePath.data(),
                importer.GetErrorString());
            return false;
        }

        m_BoundsMin = glm::vec3(std::numeric_limits<float>::max());
        m_BoundsMax = glm::vec3(std::numeric_limits<float>::lowest());

        const auto& layout = GetMeshVertexLayout();

        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            const aiMesh* mesh = scene->mMeshes[m];
            if (!mesh || mesh->mNumVertices == 0) {
                continue;
            }

            glm::vec4 baseColor(0.8f, 0.8f, 0.8f, 1.f);
            if (mesh->mMaterialIndex < scene->mNumMaterials) {
                const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
                aiColor4D diffuse{};
                if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
                    baseColor = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
                }
            }

            // Apply TexCoords & Vertice Colors To Mesh
            std::vector<MeshVertex> vertices(mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                MeshVertex& out = vertices[v];
                out.x = mesh->mVertices[v].x;
                out.y = mesh->mVertices[v].y;
                out.z = mesh->mVertices[v].z;

                m_BoundsMin = glm::min(m_BoundsMin, glm::vec3(out.x, out.y, out.z));
                m_BoundsMax = glm::max(m_BoundsMax, glm::vec3(out.x, out.y, out.z));

                if (mesh->HasNormals()) {
                    out.nx = mesh->mNormals[v].x;
                    out.ny = mesh->mNormals[v].y;
                    out.nz = mesh->mNormals[v].z;
                } else {
                    out.nx = 0.f;
                    out.ny = 1.f;
                    out.nz = 0.f;
                }

                if (mesh->HasTextureCoords(0)) {
                    out.u = mesh->mTextureCoords[0][v].x;
                    out.v = mesh->mTextureCoords[0][v].y;
                } else {
                    out.u = 0.f;
                    out.v = 0.f;
                }

                if (mesh->HasVertexColors(0)) {
                    const aiColor4D& vc = mesh->mColors[0][v];
                    out.abgr = PackColorAGBR(glm::vec4(vc.r, vc.g, vc.b, vc.a));
                } else {
                    const float light = 0.45f + 0.55f * std::clamp(out.ny * 0.5f + 0.5f, 0.f, 1.f);
                    out.abgr = PackColorAGBR(glm::vec4(baseColor.r * light, baseColor.g * light, baseColor.b * light, baseColor.a));
                }
            }

            std::vector<uint32_t> indices;
            indices.reserve(mesh->mNumFaces * 3);
            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                const aiFace& face = mesh->mFaces[f];
                for (unsigned int i = 0; i < face.mNumIndices; ++i) {
                    indices.push_back(face.mIndices[i]);
                }
            }

            SubMesh sub;
            sub.name = mesh->mName.C_Str();
            sub.baseColor = baseColor;
            sub.indexCount =  static_cast<uint32_t>(indices.size());

            sub.vbh = bgfx::createVertexBuffer(
                bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(MeshVertex))),
                layout
            );
            sub.ibh = bgfx::createIndexBuffer(
                bgfx::copy(indices.data(), static_cast<uint32_t>(indices.size() * sizeof(uint32_t))),
                BGFX_BUFFER_INDEX32
            );

            if (bgfx::isValid(sub.vbh) && bgfx::isValid(sub.ibh)) {
                m_SubMeshes.push_back(std::move(sub));
            } else {
                sub.Destroy();
            }
        }

        SDL_Log("Loaded 3D Model '%. *s' (%zu submeshes)", static_cast<int>(assetRelativePath.size()), assetRelativePath.data(), m_SubMeshes.size());
        return !m_SubMeshes.empty();
    }

    void Model::Submit(bgfx::ViewId viewId, bgfx::ProgramHandle program, const glm::mat4 &transform, uint64_t state) const {
        if (!bgfx::isValid(program) || m_SubMeshes.empty()) {
            return;
        }

        const uint64_t renderState = state != 0 ? state : BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;

        for (const auto& sub : m_SubMeshes) {
            if (!bgfx::isValid(sub.vbh) || !bgfx::isValid(sub.ibh)) {
                continue;
            }
            bgfx::setTransform(glm::value_ptr(transform));
            bgfx::setVertexBuffer(0, sub.vbh);
            bgfx::setIndexBuffer(sub.ibh);
            bgfx::setState(renderState);
            bgfx::submit(viewId, program);
        }
    }
}
