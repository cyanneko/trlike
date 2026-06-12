#pragma once

#include "World.hpp"
#include "Core/Math.hpp"
#include <memory>

// 前向声明 FastNoiseLite (如果使用)
// 当前使用自带的简单噪声实现

namespace TR {

// ============================================================
// WorldGenerator — 程序化世界生成器
// ============================================================
class WorldGenerator {
public:
    explicit WorldGenerator(uint64_t seed);

    /// 生成整个世界的初始地形 (用于首次创建)
    void GenerateWorld(World& world);

    /// 生成单个Chunk的内容
    void GenerateChunk(World& world, Chunk& chunk);

    /// 重新设置种子
    void SetSeed(uint64_t seed);

private:
    // --- 生成阶段 ---
    void GenerateTerrain(World& world, Chunk& chunk);
    void GenerateCaves(World& world, Chunk& chunk);
    void PlaceOres(World& world, Chunk& chunk);
    void FillBackgroundWalls(World& world, Chunk& chunk);

    // --- 噪声函数 ---
    /// 简单的值噪声 (后续可替换为 Perlin/Simplex)
    [[nodiscard]] double Noise2D(double x, double y) const;

    /// 用于洞穴生成的3D噪声 (x, y + 种子作为第三维)
    [[nodiscard]] double CaveNoise(double worldX, double worldY) const;

    // --- 辅助 ---
    [[nodiscard]] int GetSurfaceHeight(int worldX) const;
    [[nodiscard]] bool ShouldPlaceOre(int worldX, int worldY, double threshold, double scale) const;

    uint64_t m_seed;
    Math::XorShift64 m_rng;
};

} // namespace TR
