#include "Graphics/Camera.hpp"
#include "Core/Math.hpp"
#include <cmath>

namespace TR {

Camera::Camera() = default;

void Camera::SetPosition(sf::Vector2f pos) {
    m_position = pos;
    m_velocity = {0.f, 0.f};  // 瞬移时清零速度
}

void Camera::SetPosition(float x, float y) {
    SetPosition({x, y});
}

void Camera::SetZoom(float zoom) {
    m_zoom = Math::Clamp(zoom, 0.1f, 4.0f);
    m_targetZoom = m_zoom;
}

void Camera::ZoomIn(float factor) {
    SetTargetZoom(m_targetZoom * factor);
}

void Camera::ZoomOut(float factor) {
    SetTargetZoom(m_targetZoom * factor);
}

void Camera::SetTargetZoom(float zoom) {
    m_targetZoom = Math::Clamp(zoom, 0.1f, 4.0f);
}

void Camera::SetMoveInput(float dx, float dy) {
    m_moveInput.x = Math::Clamp(dx, -1.f, 1.f);
    m_moveInput.y = Math::Clamp(dy, -1.f, 1.f);
}

void Camera::Update(float dt) {
    // --- 缩放平滑插值 ---
    m_zoom += (m_targetZoom - m_zoom) * m_zoomSpeed * dt;
    if (std::abs(m_targetZoom - m_zoom) < 0.001f) {
        m_zoom = m_targetZoom;
    }

    // --- 移动物理 ---
    float inputMag = std::sqrt(m_moveInput.x * m_moveInput.x + m_moveInput.y * m_moveInput.y);

    if (inputMag > 0.01f) {
        // 归一化输入（防止对角线移动过快）
        sf::Vector2f inputDir = m_moveInput;
        if (inputMag > 1.0f) {
            inputDir /= inputMag;
        }

        // 加速
        m_velocity.x += inputDir.x * m_acceleration * dt;
        m_velocity.y += inputDir.y * m_acceleration * dt;

        // 限速
        float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        if (speed > m_maxSpeed) {
            float scale = m_maxSpeed / speed;
            m_velocity.x *= scale;
            m_velocity.y *= scale;
        }
    } else {
        // 无输入 → 摩擦减速
        float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        if (speed > 0.5f) {
            float frictionForce = m_friction * speed * dt;
            if (frictionForce > speed) {
                m_velocity = {0.f, 0.f};
            } else {
                float scale = 1.0f - (m_friction * dt);
                m_velocity.x *= scale;
                m_velocity.y *= scale;
            }
        } else {
            m_velocity = {0.f, 0.f};
        }
    }

    // 速度缩放（随缩放级别调整）
    float effectiveZoom = m_zoom;
    m_position.x += m_velocity.x * effectiveZoom * dt;
    m_position.y += m_velocity.y * effectiveZoom * dt;
}

sf::View Camera::GetView(const sf::RenderTarget& target) const {
    auto size = target.getSize();
    sf::Vector2f viewSize(static_cast<float>(size.x), static_cast<float>(size.y));
    sf::View view(m_position, viewSize * m_zoom);
    return view;
}

sf::Vector2f Camera::ScreenToWorld(sf::Vector2i screenPos, const sf::RenderTarget& target) const {
    auto size = target.getSize();
    sf::Vector2f viewSize(static_cast<float>(size.x), static_cast<float>(size.y));
    return m_position + sf::Vector2f(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)) * m_zoom - viewSize * 0.5f * m_zoom;
}

} // namespace TR
