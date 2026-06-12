#include "Core/Game.hpp"
#include "World/World.hpp"
#include "World/TileRegistry.hpp"
#include <iostream>
#include <chrono>

namespace TR {

Game::Game() = default;
Game::~Game() = default;

bool Game::Initialize() {
    std::cout << "TRlike - Terraria-like Game Engine\n";
    std::cout << "===================================\n\n";

    // 初始化Tile注册表
    auto& registry = TileRegistry::Instance();
    std::cout << "[Init] Tile registry loaded: " << registry.Count() << " tile types\n";

    // 创建测试世界
    World world;
    uint64_t seed = 123456789;
    world.CreateNew(8400, 2400, seed);

    std::cout << "[Init] World created: " << world.GetWidth() << "x" << world.GetHeight()
              << " tiles, seed=" << seed << "\n";
    std::cout << "[Init] Loaded chunks: " << world.GetLoadedChunkCount() << "\n\n";

    // 输出一些生成统计
    int solidCount = 0;
    int airCount = 0;
    int grassCount = 0;
    int dirtCount = 0;
    int stoneCount = 0;
    int oreCount = 0;

    world.ForEachChunk([&](ChunkPos /*cp*/, const Chunk& chunk) {
        for (int ly = 0; ly < 32; ++ly) {
            for (int lx = 0; lx < 32; ++lx) {
                const Tile& tile = chunk.GetTile(lx, ly);
                if (tile.IsAir()) {
                    airCount++;
                } else {
                    solidCount++;
                    switch (tile.type) {
                        case TILE_GRASS: grassCount++; break;
                        case TILE_DIRT:  dirtCount++; break;
                        case TILE_STONE: stoneCount++; break;
                        default:
                            if (tile.type >= TILE_ORE_COPPER && tile.type <= TILE_ORE_GOLD) {
                                oreCount++;
                            }
                            break;
                    }
                }
            }
        }
    });

    int total = solidCount + airCount;
    std::cout << "Generated chunks statistics:\n";
    std::cout << "  Total tiles: " << total << "\n";
    std::cout << "  Air: " << airCount << " (" << (100.0 * airCount / total) << "%)\n";
    std::cout << "  Solid: " << solidCount << " (" << (100.0 * solidCount / total) << "%)\n";
    std::cout << "    Grass: " << grassCount << "\n";
    std::cout << "    Dirt:  " << dirtCount << "\n";
    std::cout << "    Stone: " << stoneCount << "\n";
    std::cout << "    Ore:   " << oreCount << "\n\n";

    std::cout << "=== World generation test passed! ===\n";

    m_isRunning = true;
    return true;
}

void Game::Run() {
    if (!m_isRunning) return;

    auto lastTime = std::chrono::high_resolution_clock::now();
    double fpsAccum = 0.0;
    int fpsFrames = 0;

    std::cout << "\nGame loop starting... (press Ctrl+C to exit)\n\n";

    // 简单的控制台游戏循环 (后续替换为SFML窗口循环)
    while (m_isRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        // 限制最大步长 (防止调试断点导致的跳帧)
        if (dt > 0.1) dt = 0.1;

        // FPS统计
        fpsAccum += dt;
        fpsFrames++;
        if (fpsAccum >= 1.0) {
            m_fps = static_cast<float>(fpsFrames) / static_cast<float>(fpsAccum);
            fpsAccum = 0.0;
            fpsFrames = 0;

            // 每秒输出一次状态
            std::cout << "\r[Frame " << m_frameCount
                      << "] FPS: " << static_cast<int>(m_fps)
                      << "  DT: " << dt * 1000.0 << "ms"
                      << std::flush;
        }

        m_totalTime += dt;
        m_frameCount++;

        ProcessInput();
        Update(static_cast<float>(dt));
        Render();

        // 简单帧率控制 (在控制台模式下)
        // 后续使用SFML的垂直同步
    }
}

void Game::ProcessInput() {
    // TODO: 处理输入 (Phase 2)
}

void Game::Update(float dt) {
    // TODO: 更新游戏逻辑 (Phase 2)
    (void)dt;
}

void Game::Render() {
    // TODO: 渲染 (Phase 2)
}

void Game::Shutdown() {
    std::cout << "\n\n[Shutdown] Goodbye!\n";
    m_isRunning = false;
}

} // namespace TR
