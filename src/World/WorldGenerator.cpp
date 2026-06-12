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
    // 多层噪声叠加，产生洞穴效果
    double n1 = Noise2D(worldX * 0.03, worldY * 0.03);
    double n2 = Noise2D(worldX * 0.06 + 100.0, worldY * 0.06 + 200.0) * 0.5;
    double n3 = Noise2D(worldX * 0.12 + 300.0, worldY * 0.12 + 400.0) * 0.25;

    // 洞穴在地表以下更密集
    double depthFactor = std::max(0.0, (worldY - Constants::WORLD_SURFACE_Y) / 200.0);
    depthFactor = std::min(depthFactor, 1.0);

    return n1 + n2 + n3;
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
    // 预生成玩家出生点附近的Chunk
    int centerCX = world.GetWidth() / (2 * Constants::CHUNK_SIZE);
    int centerCY = Constants::WORLD_SURFACE_Y / Constants::CHUNK_SIZE;

    // 生成中心附近 3×3 的Chunk用于初始展示
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
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
// 阶段1：地形生成
// ============================================================
void WorldGenerator::GenerateTerrain(World& world, Chunk& chunk) {
    ChunkPos cp = chunk.GetPosition();
    BlockPos origin = chunk.GetWorldOrigin();

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;

            // 边界检查
            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);

            if (worldY < surfaceY) {
                // 地表以上 → 空气
                chunk.GetTile(lx, ly).type = TILE_AIR;
            }
            else if (worldY < surfaceY + Constants::DIRT_LAYER_DEPTH) {
                // 泥土层
                if (worldY == surfaceY) {
                    // 最顶层 = 草方块
                    chunk.GetTile(lx, ly).type = TILE_GRASS;
                } else {
                    chunk.GetTile(lx, ly).type = TILE_DIRT;
                }
            }
            else if (worldY < surfaceY + Constants::DIRT_LAYER_DEPTH + Constants::STONE_START_DEPTH) {
                // 过渡层（泥土+石头混合）
                chunk.GetTile(lx, ly).type = (worldY % 3 == 0) ? TILE_STONE : TILE_DIRT;
            }
            else {
                // 深层 = 石头
                chunk.GetTile(lx, ly).type = TILE_STONE;
            }
        }
    }
}

// ============================================================
// 阶段2：洞穴生成
// ============================================================
void WorldGenerator::GenerateCaves(World& world, Chunk& chunk) {
    ChunkPos cp = chunk.GetPosition();
    BlockPos origin = chunk.GetWorldOrigin();

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        // 只在石头层挖洞
        int surfaceY = GetSurfaceHeight(origin.x);
        if (worldY < surfaceY + Constants::DIRT_LAYER_DEPTH + Constants::STONE_START_DEPTH) {
            continue;
        }

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            double caveValue = CaveNoise(static_cast<double>(worldX),
                                         static_cast<double>(worldY));

            // 噪声值在某个范围内 → 挖空
            if (caveValue > 0.25 && caveValue < 0.55) {
                // 更大概率在较深处挖洞
                double depthChance = (worldY - surfaceY) / 600.0;
                depthChance = std::clamp(depthChance, 0.3, 1.0);

                if (std::abs(caveValue - 0.4) < 0.1 * depthChance) {
                    chunk.GetTile(lx, ly).type = TILE_AIR;
                }
            }
        }
    }
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
