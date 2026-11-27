//
// Created by sehaj on 27-11-2026.
//

#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace Garuda {

    enum class ProjectionType {
        Perspective = 0,
        Orthographic = 1
    };

    class Camera {
    public:
        static constexpr glm::vec3 WorldForward = { 1.f, 0.f, 0.f };
        static constexpr glm::vec3 WorldRight = { 0.f, 0.f, 1.f };
        static constexpr glm::vec3 WorldUp = { 0.f, 1.f, 0.f };

        Camera() = default;

        // Projection Config
        void SetPerspective(float fovDegrees, float nearClip = 0.1f, float farClip = 1000.f);
        void SetOrthographic(float orthoSize, float nearClip = -100.f, float farClip = 100.f);
        void SetAspectRatio(float aspectRatio);
        void SetViewportSize(float width, float height);

        // Transform
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& eulerDegrees);
        void LookAt(const glm::vec3& target);

        // Apply To View
        void ApplyToView(bgfx::ViewId viewId) const;

        // Getters & mutable references for ImGui Inspector
        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type);

        float& GetPerspectiveFOV() { return m_PerspectiveFOV; }
        float& GetPerspectiveNear() { return m_PerspectiveNear; }
        float& GetPerspectiveFar() { return m_PerspectiveFar; }

        float& GetOrthoSize() { return m_OrthoSize; }
        float& GetOrthoNear() { return m_OrthoNear; }
        float& GetOrthoFar() { return m_OrthoFar; }

        glm::vec3& GetPosition() { return m_Position; }
        const glm::vec3& GetPosition() const { return m_Position; }

        glm::vec3& GetRotation() { return m_Rotation; }
        const glm::vec3& GetRotation() const { return m_Rotation; }

        glm::vec3 GetForward() const;
        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

    private:
        ProjectionType m_ProjectionType = ProjectionType::Perspective;

        // Perspective Parameters
        float m_PerspectiveFOV = 60.0f;
        float m_PerspectiveNear = 0.1f;
        float m_PerspectiveFar = 1000.f;

        // Orthographic Parameters
        float m_OrthoSize = 5.f;
        float m_OrthoNear = -100.f;
        float m_OrthoFar = 100.f;

        float m_AspectRatio = 16.f / 9.f;

        // Left-handed transform (+X forward, +Y up, +Z right)
        glm::vec3 m_Position = { 0.f, 0.f, 0.f };
        glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };
    };
}