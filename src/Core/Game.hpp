#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <cstdint>
#include <memory>

namespace TR {

class World;
class Camera;
class TileRenderer;
class TextureAtlas;

// ============================================================
// Game — 游戏主类 (SFML窗口 + 游戏循环 + 世界浏览)
// ============================================================
class Game {
public:
    Game();
    ~Game();

    /// 初始化游戏 (创建窗口、加载资源、生成世界)
    bool Initialize();

    /// 运行主循环
    void Run();

    /// 退出游戏
    void Shutdown();

    /// 获取当前帧率
    [[nodiscard]] float GetFPS() const { return m_fps; }

private:
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();

    // --- SFML 窗口 ---
    sf::RenderWindow m_window;
    unsigned int m_windowWidth  = 1280;
    unsigned int m_windowHeight = 720;
    std::string m_windowTitle   = "TRlike - Map Explorer";

    // --- 核心系统 ---
    std::unique_ptr<World>        m_world;
    std::unique_ptr<Camera>       m_camera;
    std::unique_ptr<TileRenderer> m_renderer;
    std::unique_ptr<TextureAtlas> m_atlas;

    // --- 调试 UI ---
    sf::Font m_debugFont;
    sf::Text m_debugText;
    bool m_showDebug = true;

    // --- 状态 ---
    bool m_isRunning   = false;
    float m_fps        = 0.0f;
    float m_fpsTimer   = 0.0f;
    int   m_fpsFrames  = 0;
    uint64_t m_frameCount = 0;
};

} // namespace TR
