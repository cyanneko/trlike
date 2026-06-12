#include "Core/Game.hpp"
#include "World/World.hpp"
#include "World/TileRegistry.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/TileRenderer.hpp"
#include "Graphics/TextureAtlas.hpp"
#include "Core/Constants.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace TR {

Game::Game() = default;
Game::~Game() = default;

// ============================================================
// 初始化
// ============================================================
bool Game::Initialize() {
    std::cout << "TRlike - Map Explorer\n";
    std::cout << "=====================\n";

    // --- 初始化 TileRegistry ---
    auto& registry = TileRegistry::Instance();
    std::cout << "[Init] Tile types: " << registry.Count() << "\n";

    // --- 创建窗口 ---
    m_window.create(sf::VideoMode({m_windowWidth, m_windowHeight}),
                    m_windowTitle, sf::Style::Default);
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false);  // 用我们自己的长按检测
    std::cout << "[Init] Window: " << m_windowWidth << "x" << m_windowHeight << "\n";

    // --- 加载字体（使用SFML内置默认字体） ---
    // SFML 不自带字体文件，但如果系统有就能加载；没有就用空（debug文字不能显示）
    if (!m_debugFont.loadFromFile("C:\\Windows\\Fonts\\consola.ttf")) {
        // 尝试备用路径
        if (!m_debugFont.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            std::cout << "[Init] Font not found, debug text disabled\n";
            m_showDebug = false;
        }
    }
    m_debugText.setFont(m_debugFont);
    m_debugText.setCharacterSize(14);
    m_debugText.setFillColor(sf::Color::White);
    m_debugText.setOutlineColor(sf::Color::Black);
    m_debugText.setOutlineThickness(1.f);

    // --- 创建世界 ---
    m_world = std::make_unique<World>();
    uint64_t seed = 123456789;
    m_world->CreateNew(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, seed);
    std::cout << "[Init] World: " << m_world->GetWidth() << "x" << m_world->GetHeight()
              << ", seed=" << seed << "\n";

    // --- 创建相机，定位到世界出生点 ---
    m_camera = std::make_unique<Camera>();
    float spawnX = static_cast<float>(m_world->GetWidth() / 2) * Constants::TILE_SIZE;
    float spawnY = static_cast<float>(Constants::WORLD_SURFACE_Y - 10) * Constants::TILE_SIZE;
    m_camera->SetPosition(spawnX, spawnY);
    m_camera->SetZoom(1.0f);  // 默认1x缩放，看得更广

    // --- 创建纹理图集（程序化生成，无需外部图片） ---
    m_atlas = std::make_unique<TextureAtlas>();
    m_atlas->Generate();
    std::cout << "[Init] Texture atlas: " << m_atlas->GetTileSize()
              << "px tiles\n";

    // --- 创建渲染器 ---
    m_renderer = std::make_unique<TileRenderer>(*m_atlas);
    std::cout << "[Init] Renderer ready\n\n";

    std::cout << "Controls:\n";
    std::cout << "  WASD      - Move camera (hold to accelerate)\n";
    std::cout << "  Mouse Wheel - Zoom in/out\n";
    std::cout << "  F1        - Toggle debug info\n";
    std::cout << "  ESC       - Exit\n\n";

    m_isRunning = true;
    return true;
}

// ============================================================
// 退出
// ============================================================
void Game::Shutdown() {
    if (m_window.isOpen()) {
        m_window.close();
    }
    std::cout << "\n[Shutdown] Goodbye!\n";
}

// ============================================================
// 主循环
// ============================================================
void Game::Run() {
    if (!m_isRunning) return;

    sf::Clock clock;
    while (m_window.isOpen() && m_isRunning) {
        float dt = clock.restart().asSeconds();

        // 防止跳帧（如断点调试后恢复）
        if (dt > 0.1f) dt = 0.016f;

        // FPS 统计
        m_fpsTimer += dt;
        m_fpsFrames++;
        if (m_fpsTimer >= 0.5f) {
            m_fps = static_cast<float>(m_fpsFrames) / m_fpsTimer;
            m_fpsTimer = 0.f;
            m_fpsFrames = 0;
        }

        m_frameCount++;

        // 处理事件（必须在主线程）
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    m_isRunning = false;
                    break;

                case sf::Event::Resized:
                    m_windowWidth = event.size.width;
                    m_windowHeight = event.size.height;
                    // SFML 会自动调整 view
                    break;

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Key::Escape) {
                        m_isRunning = false;
                    }
                    if (event.key.code == sf::Keyboard::Key::F1) {
                        m_showDebug = !m_showDebug;
                    }
                    break;

                case sf::Event::MouseWheelScrolled:
                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                        if (event.mouseWheelScroll.delta > 0) {
                            m_camera->ZoomIn();
                        } else {
                            m_camera->ZoomOut();
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        ProcessInput(dt);
        Update(dt);
        Render();
    }
}

// ============================================================
// 输入处理 — WASD 带加速度
// ============================================================
void Game::ProcessInput(float /*dt*/) {
    float dx = 0.f;
    float dy = 0.f;

    // 连续检测键盘（支持长按加速）
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        dx -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        dx += 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        dy -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        dy += 1.f;
    }

    m_camera->SetMoveInput(dx, dy);
}

// ============================================================
// 更新
// ============================================================
void Game::Update(float dt) {
    m_camera->Update(dt);

    // 更新 debug 文字
    if (m_showDebug) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "FPS: " << static_cast<int>(m_fps) << "\n";

        auto camPos = m_camera->GetPosition();
        float ts = Constants::TILE_SIZE;
        ss << "Camera: (" << static_cast<int>(camPos.x / ts)
           << ", " << static_cast<int>(camPos.y / ts) << ") tiles\n";

        auto vel = m_camera->GetVelocity();
        ss << "Speed: " << static_cast<int>(std::sqrt(vel.x * vel.x + vel.y * vel.y))
           << " px/s\n";

        ss << "Zoom: " << std::setprecision(2) << m_camera->GetZoom() << "x\n";

        ss << "Chunks loaded: " << m_world->GetLoadedChunkCount() << "\n";

        ss << "\n[WASD] Move  [Scroll] Zoom  [F1] Info  [ESC] Quit";

        m_debugText.setString(ss.str());
    }
}

// ============================================================
// 渲染
// ============================================================
void Game::Render() {
    // 更新窗口 view
    sf::View view = m_camera->GetView(m_window);
    m_window.setView(view);

    // 清屏 — 天空色
    m_window.clear(sf::Color(64, 146, 223));  // 天蓝色背景

    // --- 渲染世界 ---
    m_renderer->Render(*m_world, *m_camera, m_window);

    // --- 渲染交叉准星（画面中心） ---
    {
        sf::Vector2f center = m_camera->GetPosition();
        constexpr float CS = 12.f;
        sf::Color cc = sf::Color(255, 255, 255, 120);
        sf::RectangleShape hLine({CS * 2.f, 1.f});
        hLine.setPosition({center.x - CS, center.y});
        hLine.setFillColor(cc);
        sf::RectangleShape vLine({1.f, CS * 2.f});
        vLine.setPosition({center.x, center.y - CS});
        vLine.setFillColor(cc);
        m_window.draw(hLine);
        m_window.draw(vLine);
    }

    // --- 渲染 Debug UI (屏幕空间) ---
    if (m_showDebug && m_debugFont.getInfo().family != "") {
        // 切换到默认视图（屏幕空间）
        m_window.setView(m_window.getDefaultView());

        // 半透明背景
        sf::RectangleShape bg({220.f, 140.f});
        bg.setPosition({8.f, 8.f});
        bg.setFillColor(sf::Color(0, 0, 0, 160));
        m_window.draw(bg);

        m_debugText.setPosition({12.f, 10.f});
        m_window.draw(m_debugText);
    }

    m_window.display();
}

} // namespace TR

