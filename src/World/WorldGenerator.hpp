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
        double thresholdStdDevScale = 1.15;
        int neighborRadius = 1;
        int samplesPerAxis = 5;
        int shallowPenaltyDepth = 32;
        double shallowPenalty = 0.16;
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
