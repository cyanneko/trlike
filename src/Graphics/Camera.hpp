#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

namespace TR {

// ============================================================
// Camera — 2D 平滑相机，支持加速度/惯性移动
// ============================================================
class Camera {
public:
    Camera();

    /// 设置相机位置 (世界坐标)
    void SetPosition(sf::Vector2f pos);
    void SetPosition(float x, float y);

    /// 获取当前位置
    [[nodiscard]] sf::Vector2f GetPosition() const { return m_position; }

    /// 获取当前速度
    [[nodiscard]] sf::Vector2f GetVelocity() const { return m_velocity; }

    /// 获取缩放
    [[nodiscard]] float GetZoom() const { return m_zoom; }

    /// 设置缩放 (滚轮)
    void SetZoom(float zoom);
    void ZoomIn(float factor = 0.9f);
    void ZoomOut(float factor = 1.1f);

    /// 目标缩放 (平滑过渡)
    void SetTargetZoom(float zoom);

    // --- 移动控制 ---
    /// 设置移动输入 (-1, 0, 或 1 表示每个方向)
    void SetMoveInput(float dx, float dy);

    /// 每帧更新 (dt 秒)
    void Update(float dt);

    /// 获取当前 sf::View (用于渲染)
    [[nodiscard]] sf::View GetView(const sf::RenderTarget& target) const;

    /// 屏幕坐标 → 世界坐标
    [[nodiscard]] sf::Vector2f ScreenToWorld(sf::Vector2i screenPos, const sf::RenderTarget& target) const;

private:
    sf::Vector2f m_position{0.f, 0.f};
    sf::Vector2f m_velocity{0.f, 0.f};
    sf::Vector2f m_moveInput{0.f, 0.f};  // -1 ~ 1 的输入

    float m_zoom = 1.0f;
    float m_targetZoom = 1.0f;

    // 移动参数
    float m_maxSpeed     = 600.f;   // 最大速度 (pixels/sec)
    float m_acceleration = 1500.f;  // 加速度 (pixels/sec²)
    float m_friction     = 3.5f;    // 摩擦系数 (越大停得越快)
    float m_zoomSpeed    = 5.0f;    // 缩放平滑速度
};

} // namespace TR
