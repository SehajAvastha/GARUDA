//
// Created by sehaj on 27-11-2026.
//

#include <Garuda/Graphics/Camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace Garuda {
    void Camera::SetPerspective(float fovDegrees, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Perspective;
        m_PerspectiveFOV = fovDegrees;
        m_PerspectiveNear = nearClip;
        m_PerspectiveFar = farClip;
    }

    void Camera::SetOrthographic(float orthoSize, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthoSize = orthoSize;
        m_OrthoNear = nearClip;
        m_OrthoFar = farClip;
    }

    void Camera::SetProjectionType(ProjectionType type) {
        m_ProjectionType = type;
    }

    void Camera::SetAspectRatio(float aspectRatio) {
        if (aspectRatio > 0.0001f) {
            m_AspectRatio = aspectRatio;
        }
    }

    void Camera::SetViewportSize(float width, float height) {
        if (height > 0.f && width > 0.f) {
            m_AspectRatio = width / height;
        }
    }

    void Camera::SetPosition(const glm::vec3 &position) {
        m_Position = position;
    }

    void Camera::SetRotation(const glm::vec3 &eulerDegrees) {
        m_Rotation.x = std::clamp(eulerDegrees.x, -89.9f, 89.9f);
        m_Rotation.y = eulerDegrees.y;
        m_Rotation.z = eulerDegrees.z;
    }

    void Camera::LookAt(const glm::vec3 &target) {
        const glm::vec3 dir = glm::normalize(target - m_Position);
        m_Rotation.x = glm::degrees(std::asin(std::clamp(dir.y, -1.f, 1.f)));
        m_Rotation.y = glm::degrees(std::atan2(dir.z, dir.x));
        m_Rotation.z = 0;
    }

    glm::vec3 Camera::GetForward() const {
        const float pitchRad = glm::radians(m_Rotation.x);
        const float yawRad = glm::radians(m_Rotation.y);

        glm::vec3 forward;
        forward.x = std::cos(pitchRad) * std::cos(yawRad);
        forward.y = std::sin(pitchRad);
        forward.z = std::cos(pitchRad) * std::sin(yawRad);
        return glm::normalize(forward);
    }

    glm::vec3 Camera::GetRight() const {
        const glm::vec3 forward = GetForward();
        const glm::vec3 baseRight = glm::normalize(glm::cross(WorldUp, forward));
        if (std::abs(m_Rotation.z) < 0.0001f) {
            return baseRight;
        }

        const glm::mat4 rollMat = glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.z), forward);
        return glm::normalize(glm::vec3(rollMat * glm::vec4(baseRight, 0.0f)));
    }

    glm::vec3 Camera::GetUp() const {
        return glm::normalize(glm::cross(GetForward(), GetRight()));
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAtLH(m_Position, m_Position + GetForward(), GetUp());
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        glm::mat4 proj(1.f);

        if (m_ProjectionType == ProjectionType::Perspective) {
            proj = glm::perspectiveLH_ZO(
                glm::radians(m_PerspectiveFOV),
                m_AspectRatio,
                m_PerspectiveNear,
                m_PerspectiveFar
            );
        } else {
            const float halfHeight = m_OrthoSize * 0.5f;
            const float halfWidth = halfHeight * m_AspectRatio;
            proj = glm::orthoLH_ZO(
                -halfWidth, halfWidth,
                -halfHeight, halfHeight,
                m_OrthoNear, m_OrthoFar
            );
        }

        // Flipping Y for GL/ES
        const bgfx::Caps* caps = bgfx::getCaps();
        if (caps && caps->originBottomLeft) {
            proj = glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f)) * proj;
        }

        return proj;
    }

    void Camera::ApplyToView(bgfx::ViewId viewId) const {
        const glm::mat4 view = GetViewMatrix();
        const glm::mat4 proj = GetProjectionMatrix();
        bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));
    }
}
