#pragma once

#include <cstdint>

namespace TR {

// ============================================================
// Game — 游戏主类 (游戏循环、状态管理)
// ============================================================
class Game {
public:
    Game();
    ~Game();

    /// 初始化游戏 (创建窗口、加载资源)
    bool Initialize();

    /// 运行主循环
    void Run();

    /// 退出游戏
    void Shutdown();

    /// 获取当前帧率
    [[nodiscard]] float GetFPS() const { return m_fps; }

    /// 获取累计运行时间 (秒)
    [[nodiscard]] double GetTotalTime() const { return m_totalTime; }

private:
    void ProcessInput();
    void Update(float dt);
    void Render();

    bool m_isRunning   = false;
    float m_fps        = 0.0f;
    double m_totalTime = 0.0;
    uint64_t m_frameCount = 0;

    // 窗口相关 (后续接入SFML)
    // sf::RenderWindow m_window;
};

} // namespace TR
