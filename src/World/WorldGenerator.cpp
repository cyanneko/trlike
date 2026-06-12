#include "World/WorldGenerator.hpp"
#include "World/World.hpp"
#include "Core/Constants.hpp"
#include <cmath>
#include <algorithm>

namespace TR {

WorldGenerator::WorldGenerator(uint64_t seed)
    : m_seed(seed)
    , m_rng(seed)
{
}

void WorldGenerator::SetSeed(uint64_t seed) {
    m_seed = seed;
    m_rng.Seed(seed);
}

// ============================================================
// 简单值噪声实现
// ============================================================
static double Fade(double t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

static double HashGrad(int ix, int iz, uint64_t seed) {
    // 基于坐标和种子的伪随机哈希
    uint64_t h = static_cast<uint64_t>(ix) * 374761393ULL;
    h = h ^ static_cast<uint64_t>(iz) * 668265263ULL;
    h = h ^ seed * 1274126177ULL;
    h = h ^ (h >> 16);
    h = h * 0x85ebca6bULL;
    h = h ^ (h >> 13);
    h = h * 0xc2b2ae35ULL;
    h = h ^ (h >> 16);
    // 映射到 [-1, 1]
    return static_cast<double>(static_cast<int64_t>(h)) / static_cast<double>(INT64_MAX);
}

static double Hash01(int ix, int iy, uint64_t seed) {
    uint64_t h = static_cast<uint64_t>(static_cast<int64_t>(ix)) * 0x9e3779b185ebca87ULL;
    h ^= static_cast<uint64_t>(static_cast<int64_t>(iy)) * 0xc2b2ae3d27d4eb4fULL;
    h ^= seed + 0x165667b19e3779f9ULL;
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    h *= 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return static_cast<double>(h >> 11) * (1.0 / 9007199254740992.0);
}

static double SmoothStep(double edge0, double edge1, double x) {
    double t = Math::Clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

static double DistanceToSegment(
    double px, double py,
    double ax, double ay,
    double bx, double by,
    double& t
) {
    double vx = bx - ax;
    double vy = by - ay;
    double lenSq = vx * vx + vy * vy;
    if (lenSq <= 0.0001) {
        t = 0.0;
        double dx = px - ax;
        double dy = py - ay;
        return std::sqrt(dx * dx + dy * dy);
    }

    t = Math::Clamp(((px - ax) * vx + (py - ay) * vy) / lenSq, 0.0, 1.0);
    double cx = ax + vx * t;
    double cy = ay + vy * t;
    double dx = px - cx;
    double dy = py - cy;
    return std::sqrt(dx * dx + dy * dy);
}

double WorldGenerator::Noise2D(double x, double y) const {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    double fx = x - ix;
    double fy = y - iy;

    double u = Fade(fx);
    double v = Fade(fy);

    // 四个角点的梯度值
    double n00 = HashGrad(ix,     iy,     m_seed);
    double n10 = HashGrad(ix + 1, iy,     m_seed);
    double n01 = HashGrad(ix,     iy + 1, m_seed);
    double n11 = HashGrad(ix + 1, iy + 1, m_seed);

    // 双线性插值 → 双三次插值
    double nx0 = n00 + (n10 - n00) * u;
    double nx1 = n01 + (n11 - n01) * u;

    return nx0 + (nx1 - nx0) * v;
}

double WorldGenerator::CaveNoise(double worldX, double worldY) const {
    double warpX = Noise2D(worldX * 0.018 + 3100.0, worldY * 0.018 + 4200.0) * 18.0
                 + Noise2D(worldX * 0.047 + 5100.0, worldY * 0.047 + 6200.0) * 5.0;
    double warpY = Noise2D(worldX * 0.018 + 7300.0, worldY * 0.018 + 8400.0) * 18.0
                 + Noise2D(worldX * 0.047 + 9300.0, worldY * 0.047 + 1400.0) * 5.0;

    double x = worldX + warpX;
    double y = worldY + warpY;

    double broad  = Noise2D(x * 0.018,          y * 0.018);
    double medium = Noise2D(x * 0.041 + 100.0,  y * 0.041 + 200.0) * 0.55;
    double fine   = Noise2D(x * 0.085 + 300.0,  y * 0.085 + 400.0) * 0.28;
    double vein   = Noise2D(x * 0.020 + 900.0,  y * 0.052 + 1100.0) * 0.16;

    return (broad + medium + fine + vein) / 1.99;
}

// ============================================================
// 地表高度计算
// ============================================================
int WorldGenerator::GetSurfaceHeight(int worldX) const {
    // 使用噪声生成起伏的地表
    double n = Noise2D(worldX * 0.005, 0.0);
    // 再加一个大尺度噪声做山脉
    double n2 = Noise2D(worldX * 0.001 + 500.0, 100.0) * 0.5;

    int baseHeight = Constants::WORLD_SURFACE_Y;
    int variation = static_cast<int>((n + n2) * 80.0); // ±80 tiles 的高度变化
    return baseHeight + variation;
}

// ============================================================
// 矿物判定
// ============================================================
bool WorldGenerator::ShouldPlaceOre(int worldX, int worldY, double threshold, double scale) const {
    double n = Noise2D(worldX * scale, worldY * scale);
    return n > threshold;
}

// ============================================================
// 生成整个World
// ============================================================
void WorldGenerator::GenerateWorld(World& world) {
    // 预生成玩家出生点附近的 Chunk（7×7 网格确保初始视野有内容）
    int centerCX = world.GetWidth() / (2 * Constants::CHUNK_SIZE);
    int centerCY = Constants::WORLD_SURFACE_Y / Constants::CHUNK_SIZE;

    for (int dy = -3; dy <= 3; ++dy) {
        for (int dx = -3; dx <= 3; ++dx) {
            ChunkPos cp{centerCX + dx, centerCY + dy};
            auto [it, _] = world.m_chunks.emplace(cp, Chunk(cp));
            GenerateChunk(world, it->second);
        }
    }
}

// ============================================================
// 生成单个Chunk
// ============================================================
void WorldGenerator::GenerateChunk(World& world, Chunk& chunk) {
    ChunkPos cp = chunk.GetPosition();

    // 按阶段生成
    GenerateTerrain(world, chunk);
    GenerateCaves(world, chunk);
    PlaceOres(world, chunk);
    FillBackgroundWalls(world, chunk);

    chunk.MarkClean(); // 生成完成，标记为干净
}

// ============================================================
// 阶段1：地形生成（噪声驱动泥土→石头自然过渡）
// ============================================================
void WorldGenerator::GenerateTerrain(World& world, Chunk& chunk) {
    ChunkPos cp = chunk.GetPosition();
    BlockPos origin = chunk.GetWorldOrigin();

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;

            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);

            // 地表以上 → 空气
            if (worldY < surfaceY) {
                chunk.GetTile(lx, ly).type = TILE_AIR;
                continue;
            }

            // 地表 = 草方块
            if (worldY == surfaceY) {
                chunk.GetTile(lx, ly).type = TILE_GRASS;
                continue;
            }

            int depth = worldY - surfaceY;

            // --- 噪声驱动的过渡带 ---
            // 过渡带起始深度：地表下 8~18 格（不同水平位置不一样）
            double transNoise = Noise2D(worldX * 0.03, 700.0);
            int transStart = 8 + static_cast<int>((transNoise + 1.0) * 5.0);   // [8, 18]
            int transEnd   = transStart + 18;                                   // 过渡带厚 18 格

            if (depth < transStart) {
                // === 泥土层 ===
                // 在泥土层底部偶尔出现小石块（模拟自然夹杂）
                double stoneSpot = Noise2D(worldX * 0.25, worldY * 0.25 + 300.0);
                if (depth >= transStart - 5 && stoneSpot > 0.72) {
                    chunk.GetTile(lx, ly).type = TILE_STONE;
                } else {
                    chunk.GetTile(lx, ly).type = TILE_DIRT;
                }
            }
            else if (depth < transEnd) {
                // === 过渡层：石头比例随深度递增 ===
                double t = static_cast<double>(depth - transStart)
                         / static_cast<double>(transEnd - transStart);  // 0→1

                // 用噪声扰动石头出现概率，产生斑驳感
                double n = Noise2D(worldX * 0.07, worldY * 0.07 + 1000.0);
                double stoneChance = t + n * 0.35;  // 噪声 ±35% 扰动
                stoneChance = Math::Clamp(stoneChance, 0.0, 1.0);

                // 确定性判定（用坐标哈希代替随机数，保证可复现）
                double hash = Noise2D(worldX * 0.5, worldY * 0.5 + 2000.0);
                if ((hash * 0.5 + 0.5) < stoneChance) {
                    chunk.GetTile(lx, ly).type = TILE_STONE;
                } else {
                    chunk.GetTile(lx, ly).type = TILE_DIRT;
                }
            }
            else {
                // === 深层 = 纯石头 ===
                chunk.GetTile(lx, ly).type = TILE_STONE;
            }
        }
    }
}

// ============================================================
// 阶段2：洞穴生成（蛇形隧道 + 洞室膨胀 + 天然地表突破）
// ============================================================
void WorldGenerator::GenerateCaves(World& world, Chunk& chunk) {
    BlockPos origin = chunk.GetWorldOrigin();

    auto canCarve = [](TileID type) {
        return type == TILE_DIRT || type == TILE_STONE;
    };

    auto carveTile = [&](int lx, int ly) {
        Tile& tile = chunk.GetTile(lx, ly);
        if (!canCarve(tile.type)) return;
        tile.type = TILE_AIR;
        tile.wall = WALL_NONE;
        tile.liquid = 0;
    };

    // --- 第一遍：扭曲噪声形成细长洞脉，深处更密集 ---
    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);
            int depth = worldY - surfaceY;

            if (depth < 8) continue;  // 留出稳定表土层

            TileID curType = chunk.GetTile(lx, ly).type;
            if (!canCarve(curType)) continue;

            double caveValue = CaveNoise(static_cast<double>(worldX),
                                         static_cast<double>(worldY));

            double depthOpen = SmoothStep(14.0, 320.0, static_cast<double>(depth));
            double cavernBand = Noise2D(worldX * 0.013 + 12300.0, worldY * 0.013 + 12700.0);
            double threshold = 0.026 + depthOpen * 0.105;
            threshold += std::max(0.0, cavernBand - 0.28) * 0.11;
            threshold += SmoothStep(420.0, 1000.0, static_cast<double>(depth)) * 0.035;

            double raggedEdge = Noise2D(worldX * 0.22 + 13100.0, worldY * 0.22 + 13700.0) * 0.024;
            threshold = Math::Clamp(threshold + raggedEdge, 0.018, 0.205);

            if (std::abs(caveValue) < threshold) {
                carveTile(lx, ly);
            }
        }
    }

    // --- 第二遍：大洞室。它们不连续刷满，而是作为通道网络里的膨大节点 ---
    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);
            int depth = worldY - surfaceY;
            if (depth < 45) continue;

            TileID curType = chunk.GetTile(lx, ly).type;
            if (!canCarve(curType)) continue;

            double depthOpen = SmoothStep(60.0, 520.0, static_cast<double>(depth));
            double chamber = Noise2D(worldX * 0.010 + 17100.0, worldY * 0.010 + 18100.0) * 0.78
                           + Noise2D(worldX * 0.021 + 17300.0, worldY * 0.021 + 18300.0) * 0.34
                           + Noise2D(worldX * 0.052 + 17500.0, worldY * 0.052 + 18500.0) * 0.14;
            double chamberThreshold = 0.66 - depthOpen * 0.13;

            if (chamber > chamberThreshold) {
                double edgeBreakup = Noise2D(worldX * 0.18 + 19100.0, worldY * 0.18 + 19700.0);
                if (edgeBreakup > -0.55) {
                    carveTile(lx, ly);
                }
            }
        }
    }

    // --- 第三遍：确定性蛇形通道。附近网格种子决定一条弯曲 polyline 是否经过此 tile ---
    constexpr int wormCellW = 128;
    constexpr int wormCellH = 96;
    constexpr int wormSegments = 9;

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);
            int depth = worldY - surfaceY;
            if (depth < 16) continue;

            TileID curType = chunk.GetTile(lx, ly).type;
            if (!canCarve(curType)) continue;

            int baseCellX = static_cast<int>(std::floor(static_cast<double>(worldX) / wormCellW));
            int baseCellY = static_cast<int>(std::floor(static_cast<double>(worldY) / wormCellH));
            bool wormCarve = false;

            for (int cellY = baseCellY - 2; cellY <= baseCellY + 2 && !wormCarve; ++cellY) {
                for (int cellX = baseCellX - 2; cellX <= baseCellX + 2 && !wormCarve; ++cellX) {
                    double seedX = cellX * wormCellW + 12.0
                                 + Hash01(cellX, cellY, m_seed + 17ULL) * (wormCellW - 24.0);
                    double seedY = cellY * wormCellH + 10.0
                                 + Hash01(cellX, cellY, m_seed + 37ULL) * (wormCellH - 20.0);

                    if (seedX < 0.0 || seedX >= static_cast<double>(world.GetWidth())) continue;

                    int seedSurface = GetSurfaceHeight(static_cast<int>(std::round(seedX)));
                    double seedDepth = seedY - seedSurface;
                    if (seedDepth < 18.0) continue;

                    double activeChance = 0.15
                                        + SmoothStep(60.0, 360.0, seedDepth) * 0.24
                                        + SmoothStep(520.0, 1100.0, seedDepth) * 0.08;
                    if (Hash01(cellX, cellY, m_seed + 53ULL) > activeChance) continue;

                    double dirSign = Hash01(cellX, cellY, m_seed + 71ULL) < 0.5 ? -1.0 : 1.0;
                    double angle = (Hash01(cellX, cellY, m_seed + 89ULL) - 0.5) * 1.35;
                    double dirX = std::cos(angle) * dirSign;
                    double dirY = std::sin(angle) * 0.9;
                    double perpX = -dirY;
                    double perpY = dirX;

                    double length = 82.0
                                  + Hash01(cellX, cellY, m_seed + 107ULL) * 132.0
                                  + SmoothStep(220.0, 900.0, seedDepth) * 36.0;
                    double amp = 10.0 + Hash01(cellX, cellY, m_seed + 131ULL) * 22.0;
                    double phase = Hash01(cellX, cellY, m_seed + 151ULL) * Math::Pi<double> * 2.0;
                    double baseRadius = 2.2
                                      + Hash01(cellX, cellY, m_seed + 173ULL) * 2.5
                                      + SmoothStep(120.0, 620.0, seedDepth) * 1.25;
                    double knotT = 0.22 + Hash01(cellX, cellY, m_seed + 191ULL) * 0.56;
                    double knotSize = 1.5 + Hash01(cellX, cellY, m_seed + 211ULL) * 4.5;

                    double px[wormSegments + 1]{};
                    double py[wormSegments + 1]{};
                    for (int s = 0; s <= wormSegments; ++s) {
                        double t = static_cast<double>(s) / static_cast<double>(wormSegments);
                        double wave = std::sin(t * Math::Pi<double> * 2.0 + phase) * amp;
                        wave += Noise2D(seedX * 0.012 + t * 4.0 + 21100.0,
                                        seedY * 0.012 + phase + 21700.0) * amp * 0.45;

                        px[s] = seedX + dirX * length * t + perpX * wave;
                        py[s] = seedY + dirY * length * t + perpY * wave;
                    }

                    for (int s = 0; s < wormSegments; ++s) {
                        double segmentT = 0.0;
                        double dist = DistanceToSegment(
                            static_cast<double>(worldX), static_cast<double>(worldY),
                            px[s], py[s], px[s + 1], py[s + 1], segmentT
                        );

                        double pathT = (static_cast<double>(s) + segmentT)
                                     / static_cast<double>(wormSegments);
                        double taper = 0.66 + std::sin(pathT * Math::Pi<double>) * 0.44;
                        double radius = baseRadius * taper;

                        double knot = std::max(0.0, 1.0 - std::abs(pathT - knotT) / 0.16);
                        radius += knot * knotSize;
                        radius += Noise2D(worldX * 0.19 + seedX * 0.013,
                                          worldY * 0.19 + seedY * 0.013) * 0.55;

                        if (dist < radius) {
                            wormCarve = true;
                            break;
                        }
                    }
                }
            }

            if (wormCarve) {
                carveTile(lx, ly);
            }
        }
    }

    // --- 第四遍：地表入口，直接打穿草皮接入地下通道 ---
    auto canCarveEntrance = [](TileID type) {
        return type == TILE_GRASS || type == TILE_DIRT || type == TILE_STONE;
    };

    auto carveEntranceTile = [&](int lx, int ly) {
        Tile& tile = chunk.GetTile(lx, ly);
        if (!canCarveEntrance(tile.type)) return;
        tile.type = TILE_AIR;
        tile.wall = WALL_NONE;
        tile.liquid = 0;
    };

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);
            int depth = worldY - surfaceY;
            if (depth < 0 || depth > 110) continue;

            TileID curType = chunk.GetTile(lx, ly).type;
            if (!canCarveEntrance(curType)) continue;

            constexpr int entranceSpacing = 180;
            int entranceIdx = static_cast<int>(std::round(static_cast<double>(worldX) / entranceSpacing));
            int entranceX = entranceIdx * entranceSpacing;
            if (entranceX < 0 || entranceX >= world.GetWidth()) continue;

            int spawnEntranceIdx = static_cast<int>(std::round(
                static_cast<double>(world.GetWidth() / 2) / entranceSpacing
            ));
            bool forcedSpawnEntrance = entranceIdx == spawnEntranceIdx;
            if (!forcedSpawnEntrance && Hash01(entranceIdx, 0, m_seed + 251ULL) > 0.62) continue;

            int entranceSurface = GetSurfaceHeight(entranceX);
            int entranceDepth = worldY - entranceSurface;
            if (entranceDepth < 0 || entranceDepth > 112) continue;

            double dir = Hash01(entranceIdx, 0, m_seed + 269ULL) < 0.5 ? -1.0 : 1.0;
            double angleDeg = 28.0 + Hash01(entranceIdx, 0, m_seed + 283ULL) * 22.0;
            double angleRad = angleDeg * Math::Pi<double> / 180.0;
            double t = static_cast<double>(entranceDepth) / 112.0;
            double centerX = static_cast<double>(entranceX)
                           + dir * entranceDepth / std::tan(angleRad)
                           + std::sin(entranceDepth * 0.105 + entranceIdx * 1.37) * 5.5
                           + Noise2D(entranceDepth * 0.045, entranceIdx * 0.21 + 23100.0) * 4.5;
            double width = 1.4 + t * 2.4
                         + Noise2D(worldX * 0.16 + 24100.0, worldY * 0.16 + 24700.0) * 0.65;

            double localDepth = static_cast<double>(depth);
            double mouthDepth = 16.0 + Hash01(entranceIdx, 0, m_seed + 307ULL) * 12.0;
            double mouthBlend = SmoothStep(0.0, mouthDepth, localDepth);
            double surfaceMouthCenter = static_cast<double>(entranceX)
                                      + (Hash01(entranceIdx, 0, m_seed + 331ULL) - 0.5) * 5.5;
            double mouthCenter = Math::Lerp(surfaceMouthCenter, centerX, mouthBlend)
                               + Noise2D(localDepth * 0.20 + 25100.0,
                                         entranceIdx * 0.31 + 25700.0) * (1.8 - mouthBlend * 0.9);
            double topWidth = 2.0 + Hash01(entranceIdx, 0, m_seed + 353ULL) * 2.4;
            double lowerWidth = Math::Clamp(width + 0.9, 2.4, 5.3);
            double mouthWidth = Math::Lerp(topWidth, lowerWidth, mouthBlend)
                              + Noise2D(worldX * 0.34 + 26100.0,
                                        worldY * 0.28 + 26700.0) * 0.75;
            double edgeRoughness = Noise2D(worldX * 0.48 + 27100.0,
                                           worldY * 0.36 + 27700.0) * 0.85;

            bool mouth = localDepth <= mouthDepth
                      && std::abs(static_cast<double>(worldX) - mouthCenter) + edgeRoughness
                       < Math::Clamp(mouthWidth, 1.7, 5.6);
            bool shaft = entranceDepth > mouthDepth * 0.55
                      && std::abs(static_cast<double>(worldX) - centerX) + edgeRoughness * 0.45
                       < Math::Clamp(width, 1.5, 5.0);

            if (mouth || shaft) {
                carveEntranceTile(lx, ly);
            }
        }
    }

    // --- 第五遍：清理 1 格高横缝和孤立噪点 ---
    bool toFill[Constants::CHUNK_SIZE][Constants::CHUNK_SIZE]{};

    for (int ly = 1; ly < Constants::CHUNK_SIZE - 1; ++ly) {
        for (int lx = 1; lx < Constants::CHUNK_SIZE - 1; ++lx) {
            if (!chunk.GetTile(lx, ly).IsAir()) continue;

            bool solidAbove = chunk.GetTile(lx, ly - 1).IsSolid();
            bool solidBelow = chunk.GetTile(lx, ly + 1).IsSolid();

            int airNeighbors = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if ((dx != 0 || dy != 0) && !chunk.GetTile(lx + dx, ly + dy).IsSolid())
                        airNeighbors++;

            if ((solidAbove && solidBelow) || airNeighbors <= 1) {
                toFill[ly][lx] = true;
            }
        }
    }

    for (int ly = 1; ly < Constants::CHUNK_SIZE - 1; ++ly)
        for (int lx = 1; lx < Constants::CHUNK_SIZE - 1; ++lx)
            if (toFill[ly][lx])
                chunk.GetTile(lx, ly).type = TILE_STONE;

    // --- 第六遍：轻度扩张边缘，使隧道更像被挖开的自然洞，而不是细线 ---
    bool toExpand[Constants::CHUNK_SIZE][Constants::CHUNK_SIZE]{};

    for (int ly = 2; ly < Constants::CHUNK_SIZE - 2; ++ly) {
        for (int lx = 2; lx < Constants::CHUNK_SIZE - 2; ++lx) {
            if (!chunk.GetTile(lx, ly).IsSolid()) continue;

            int airNeighbors = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if ((dx != 0 || dy != 0) && !chunk.GetTile(lx + dx, ly + dy).IsSolid())
                        airNeighbors++;

            int worldX = origin.x + lx;
            int worldY = origin.y + ly;
            double edgeNoise = Noise2D(worldX * 0.31 + 28100.0, worldY * 0.31 + 28700.0);

            if (airNeighbors >= 6 || (airNeighbors == 5 && edgeNoise > -0.10)) {
                toExpand[ly][lx] = true;
            }
        }
    }
    for (int ly = 2; ly < Constants::CHUNK_SIZE - 2; ++ly)
        for (int lx = 2; lx < Constants::CHUNK_SIZE - 2; ++lx)
            if (toExpand[ly][lx])
                chunk.GetTile(lx, ly).type = TILE_AIR;
}

// ============================================================
// 阶段3：矿物放置
// ============================================================
void WorldGenerator::PlaceOres(World& world, Chunk& chunk) {
    ChunkPos cp = chunk.GetPosition();
    BlockPos origin = chunk.GetWorldOrigin();

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            Tile& tile = chunk.GetTile(lx, ly);
            // 只在石头上放置矿物
            if (tile.type != TILE_STONE) continue;

            int depth = worldY - GetSurfaceHeight(worldX);
            if (depth < 10) continue;  // 太浅不生成矿物

            // 铜矿 - 浅层常见
            if (depth < 300 && ShouldPlaceOre(worldX, worldY, 0.92, 0.08)) {
                tile.type = TILE_ORE_COPPER;
            }
            // 铁矿 - 中层
            else if (depth > 50 && depth < 600 && ShouldPlaceOre(worldX, worldY, 0.94, 0.06)) {
                tile.type = TILE_ORE_IRON;
            }
            // 银矿 - 中层
            else if (depth > 100 && depth < 700 && ShouldPlaceOre(worldX, worldY, 0.96, 0.05)) {
                tile.type = TILE_ORE_SILVER;
            }
            // 金矿 - 深层
            else if (depth > 200 && ShouldPlaceOre(worldX, worldY, 0.97, 0.04)) {
                tile.type = TILE_ORE_GOLD;
            }
        }
    }
}

// ============================================================
// 阶段4：背景墙填充
// ============================================================
void WorldGenerator::FillBackgroundWalls(World& world, Chunk& chunk) {
    ChunkPos cp = chunk.GetPosition();
    BlockPos origin = chunk.GetWorldOrigin();

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);

            // 地下有泥土/石头的地方填充土墙
            if (worldY > surfaceY + 3) {
                Tile& tile = chunk.GetTile(lx, ly);
                if (tile.IsSolid() && tile.wall == WALL_NONE) {
                    // 80%概率生成背景墙
                    double n = Noise2D(worldX * 0.1, worldY * 0.1);
                    if (n > -0.3) {
                        if (tile.type == TILE_DIRT || tile.type == TILE_GRASS) {
                            tile.wall = WALL_DIRT;
                        } else if (tile.type == TILE_STONE) {
                            tile.wall = WALL_STONE;
                        }
                    }
                }
            }
        }
    }
}

} // namespace TR
