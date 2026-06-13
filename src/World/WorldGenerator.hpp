#pragma once

#include "World.hpp"
#include "Core/Math.hpp"
#include <cstdint>

namespace TR {

class WorldGenerator {
public:
    explicit WorldGenerator(uint64_t seed);

    void GenerateWorld(World& world);
    void GenerateChunk(World& world, Chunk& chunk);
    void SetSeed(uint64_t seed);

private:
    struct Noise1DSettings {
        double frequency = 1.0;
        double amplitude = 1.0;
        int octaves = 1;
        double lacunarity = 2.0;
        double persistence = 0.5;
    };

    struct Noise2DSettings {
        double frequency = 1.0;
        double amplitude = 1.0;
        int octaves = 1;
        double lacunarity = 2.0;
        double persistence = 0.5;
        double rotationRadians = 0.0;
    };

    struct CaveGenerationSettings {
        // > 1.0 creates more cave tiles by lowering the carve threshold.
        double density = 1.08;
        // > 1.0 makes the main cave field lower-frequency, producing larger caverns.
        double size = 0.95;
        // Higher values make cave edges more broken and irregular.
        double edgeRoughness = 1.10;
        // 0.0 keeps pure noise clustering, 1.0 strongly evens cave coverage across the map.
        double uniformity = 0.90;
        // The logical tile size of each distribution region that receives a cave anchor.
        int distributionCellSize = 120;
        // Tiles below this coverage receive a soft penalty instead of being blocked.
        double minimumAnchorCoverage = 0.10;
        // Controls how tightly each region's cave cluster stays around its anchor.
        double anchorInnerRadius = 0.12;
        double anchorOuterRadius = 0.62;
        // Higher values preserve more solid separators between neighboring clusters.
        double separationStrength = 0.48;
        // Adapts each chunk threshold toward a target local A-B average.
        double adaptiveThresholdTarget = 0.20;
        double adaptiveThresholdStrength = 0.55;
        double adaptiveThresholdMaxOffset = 0.08;
        int adaptiveThresholdNeighborhood = 1;
        int adaptiveThresholdSamplesPerAxis = 4;
        // Equalizes cave score per distribution cell to reduce empty or over-dense regions.
        double regionalEqualizationTarget = 0.22;
        double regionalEqualizationStrength = 0.70;
        double regionalEqualizationMaxOffset = 0.075;
        int regionalEqualizationSamplesPerAxis = 4;
        // Keeps a stable soil layer before caves are allowed to carve.
        int surfaceProtectionDepth = 12;
    };

    inline static constexpr CaveGenerationSettings kCaveSettings{};

    void GenerateTerrain(World& world, Chunk& chunk);
    void GenerateCaves(World& world, Chunk& chunk);

    [[nodiscard]] double Perlin1D(double x, uint64_t salt) const;
    [[nodiscard]] double Perlin2D(double x, double y, uint64_t salt) const;
    [[nodiscard]] double FractalPerlin1D(double x, const Noise1DSettings& settings, uint64_t salt) const;
    [[nodiscard]] double FractalPerlin2D(double x, double y, const Noise2DSettings& settings, uint64_t salt) const;

    [[nodiscard]] int GetSurfaceHeight(int worldX) const;
    [[nodiscard]] int ToLogicalX(int worldX) const;
    [[nodiscard]] int ToLogicalY(int worldY) const;

    uint64_t m_seed;
    Math::XorShift64 m_rng;
};

} // namespace TR
