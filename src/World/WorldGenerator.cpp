#include "World/WorldGenerator.hpp"
#include "World/World.hpp"
#include "Core/Constants.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace TR {

namespace {

constexpr uint64_t kHashPrimeX = 0x9e3779b185ebca87ULL;
constexpr uint64_t kHashPrimeY = 0xc2b2ae3d27d4eb4fULL;
constexpr uint64_t kHashPrimeSeed = 0x165667b19e3779f9ULL;

double Fade(double t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

uint64_t Mix64(uint64_t value) {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31;
    return value;
}

uint64_t HashCoord(int x, int y, uint64_t seed, uint64_t salt) {
    uint64_t h = static_cast<uint64_t>(static_cast<int64_t>(x)) * kHashPrimeX;
    h ^= static_cast<uint64_t>(static_cast<int64_t>(y)) * kHashPrimeY;
    h ^= (seed + salt * kHashPrimeSeed);
    return Mix64(h);
}

double Hash01(int x, int y, uint64_t seed, uint64_t salt) {
    return static_cast<double>(HashCoord(x, y, seed, salt) >> 11)
         * (1.0 / 9007199254740992.0);
}

double PositiveNoise(double noise) {
    return Math::Clamp(noise * 0.5 + 0.5, 0.0, 1.0);
}

double SmoothStep(double edge0, double edge1, double value) {
    double t = Math::Clamp((value - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

bool CanCaveCarve(TileID type) {
    return type == TILE_DIRT || type == TILE_STONE;
}

} // namespace

WorldGenerator::WorldGenerator(uint64_t seed)
    : m_seed(seed)
    , m_rng(seed)
{
}

void WorldGenerator::SetSeed(uint64_t seed) {
    m_seed = seed;
    m_rng.Seed(seed);
}

int WorldGenerator::ToLogicalX(int worldX) const {
    return worldX + Constants::WORLD_MIN_X;
}

int WorldGenerator::ToLogicalY(int worldY) const {
    return worldY + Constants::WORLD_MIN_Y;
}

double WorldGenerator::Perlin1D(double x, uint64_t salt) const {
    int ix = static_cast<int>(std::floor(x));
    double fx = x - static_cast<double>(ix);

    auto gradient = [&](int latticeX) {
        return (HashCoord(latticeX, 0, m_seed, salt) & 1ULL) == 0ULL ? -1.0 : 1.0;
    };

    double n0 = gradient(ix) * fx;
    double n1 = gradient(ix + 1) * (fx - 1.0);
    return Math::Lerp(n0, n1, Fade(fx)) * 2.0;
}

double WorldGenerator::Perlin2D(double x, double y, uint64_t salt) const {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    double fx = x - static_cast<double>(ix);
    double fy = y - static_cast<double>(iy);

    auto dotGradient = [&](int latticeX, int latticeY, double dx, double dy) {
        double angle = Hash01(latticeX, latticeY, m_seed, salt)
                     * Math::Pi<double> * 2.0;
        double gx = std::cos(angle);
        double gy = std::sin(angle);
        return gx * dx + gy * dy;
    };

    double n00 = dotGradient(ix,     iy,     fx,       fy);
    double n10 = dotGradient(ix + 1, iy,     fx - 1.0, fy);
    double n01 = dotGradient(ix,     iy + 1, fx,       fy - 1.0);
    double n11 = dotGradient(ix + 1, iy + 1, fx - 1.0, fy - 1.0);

    double u = Fade(fx);
    double v = Fade(fy);
    double nx0 = Math::Lerp(n00, n10, u);
    double nx1 = Math::Lerp(n01, n11, u);

    return Math::Clamp(Math::Lerp(nx0, nx1, v) * 1.41421356237, -1.0, 1.0);
}

double WorldGenerator::FractalPerlin1D(
    double x,
    const Noise1DSettings& settings,
    uint64_t salt
) const {
    double value = 0.0;
    double amplitude = 1.0;
    double frequency = settings.frequency;
    double amplitudeSum = 0.0;

    for (int octave = 0; octave < settings.octaves; ++octave) {
        value += Perlin1D(x * frequency, salt + static_cast<uint64_t>(octave) * 101ULL) * amplitude;
        amplitudeSum += amplitude;
        frequency *= settings.lacunarity;
        amplitude *= settings.persistence;
    }

    if (amplitudeSum <= 0.0) return 0.0;
    return (value / amplitudeSum) * settings.amplitude;
}

double WorldGenerator::FractalPerlin2D(
    double x,
    double y,
    const Noise2DSettings& settings,
    uint64_t salt
) const {
    double cosR = std::cos(settings.rotationRadians);
    double sinR = std::sin(settings.rotationRadians);
    double rx = x * cosR - y * sinR;
    double ry = x * sinR + y * cosR;

    double value = 0.0;
    double amplitude = 1.0;
    double frequency = settings.frequency;
    double amplitudeSum = 0.0;

    for (int octave = 0; octave < settings.octaves; ++octave) {
        value += Perlin2D(rx * frequency, ry * frequency,
                          salt + static_cast<uint64_t>(octave) * 131ULL) * amplitude;
        amplitudeSum += amplitude;
        frequency *= settings.lacunarity;
        amplitude *= settings.persistence;
    }

    if (amplitudeSum <= 0.0) return 0.0;
    return (value / amplitudeSum) * settings.amplitude;
}

int WorldGenerator::GetSurfaceHeight(int worldX) const {
    double x = static_cast<double>(ToLogicalX(worldX));

    double wave = 0.0;
    wave += std::sin((x + Hash01(1, 0, m_seed, 11ULL) * 900.0) * Math::Pi<double> * 2.0 / 920.0) * 38.0;
    wave += std::sin((x + Hash01(2, 0, m_seed, 13ULL) * 430.0) * Math::Pi<double> * 2.0 / 430.0) * 18.0;
    wave += std::sin((x + Hash01(3, 0, m_seed, 17ULL) * 180.0) * Math::Pi<double> * 2.0 / 180.0) * 7.0;

    Noise1DSettings broadNoise{
        .frequency = 0.0045,
        .amplitude = 30.0,
        .octaves = 4,
        .lacunarity = 2.0,
        .persistence = 0.48
    };
    Noise1DSettings fineNoise{
        .frequency = 0.019,
        .amplitude = 8.0,
        .octaves = 3,
        .lacunarity = 2.15,
        .persistence = 0.42
    };

    double noise = FractalPerlin1D(x, broadNoise, 1001ULL)
                 + FractalPerlin1D(x, fineNoise, 1009ULL);

    int surface = Constants::WORLD_SURFACE_Y + static_cast<int>(std::round(wave + noise));
    return Math::Clamp(surface,
                       Constants::WORLD_SURFACE_Y - 130,
                       Constants::WORLD_SURFACE_Y + 110);
}

void WorldGenerator::GenerateWorld(World& world) {
    int centerCX = world.GetWidth() / (2 * Constants::CHUNK_SIZE);
    int centerCY = Constants::WORLD_SURFACE_Y / Constants::CHUNK_SIZE;

    for (int dy = -3; dy <= 3; ++dy) {
        for (int dx = -3; dx <= 3; ++dx) {
            ChunkPos cp{centerCX + dx, centerCY + dy};
            BlockPos origin = cp.ToBlockPos(Constants::CHUNK_SIZE);
            if (origin.x >= world.GetWidth() || origin.y >= world.GetHeight()) continue;
            if (origin.x + Constants::CHUNK_SIZE <= 0 || origin.y + Constants::CHUNK_SIZE <= 0) continue;

            auto [it, inserted] = world.m_chunks.emplace(cp, Chunk(cp));
            if (inserted) {
                GenerateChunk(world, it->second);
            }
        }
    }
}

void WorldGenerator::GenerateChunk(World& world, Chunk& chunk) {
    GenerateTerrain(world, chunk);
    GenerateCaves(world, chunk);
    chunk.MarkClean();
}

void WorldGenerator::GenerateTerrain(World& world, Chunk& chunk) {
    BlockPos origin = chunk.GetWorldOrigin();

    Noise2DSettings dirtNoise{
        .frequency = 0.055,
        .amplitude = 7.0,
        .octaves = 3,
        .lacunarity = 2.0,
        .persistence = 0.52,
        .rotationRadians = 0.31
    };
    Noise2DSettings dirtPocketNoise{
        .frequency = 0.105,
        .amplitude = 1.0,
        .octaves = 2,
        .lacunarity = 2.1,
        .persistence = 0.45,
        .rotationRadians = -0.42
    };

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            Tile& tile = chunk.GetTile(lx, ly);
            tile.Clear();

            if (!world.IsInBounds({worldX, worldY})) continue;

            int surfaceY = GetSurfaceHeight(worldX);
            if (worldY < surfaceY) continue;

            int logicalX = ToLogicalX(worldX);
            int logicalY = ToLogicalY(worldY);
            int depth = worldY - surfaceY;

            tile.type = TILE_STONE;

            double irregularDepth = 13.0
                                  + FractalPerlin2D(static_cast<double>(logicalX),
                                                    static_cast<double>(logicalY),
                                                    dirtNoise,
                                                    2003ULL);
            double pocket = PositiveNoise(FractalPerlin2D(static_cast<double>(logicalX),
                                                          static_cast<double>(logicalY),
                                                          dirtPocketNoise,
                                                          2011ULL));

            if (static_cast<double>(depth) <= irregularDepth || (depth <= 24 && pocket > 0.68)) {
                tile.type = TILE_DIRT;
            }

            if (depth == 0 && tile.type == TILE_DIRT) {
                tile.type = TILE_GRASS;
            }
        }
    }
}

void WorldGenerator::GenerateCaves(World& world, Chunk& chunk) {
    BlockPos origin = chunk.GetWorldOrigin();
    ChunkPos chunkPos = chunk.GetPosition();
    const CaveGenerationSettings& caveSettings = kCaveSettings;
    const int distributionCellSize = std::max(48, caveSettings.distributionCellSize);
    const double uniformity = Math::Clamp(caveSettings.uniformity, 0.0, 1.0);
    const double caveSize = std::max(0.35, caveSettings.size);
    const double minimumAnchorCoverage = Math::Clamp(caveSettings.minimumAnchorCoverage, 0.0, 0.85);
    const double anchorInnerRadius = Math::Clamp(caveSettings.anchorInnerRadius, 0.02, 0.45);
    const double anchorOuterRadius = std::max(anchorInnerRadius + 0.05, caveSettings.anchorOuterRadius);
    const double separationStrength = Math::Clamp(caveSettings.separationStrength, 0.0, 1.0);
    const int adaptiveNeighborhood = std::max(0, caveSettings.adaptiveThresholdNeighborhood);
    const int adaptiveSamplesPerAxis = std::max(1, caveSettings.adaptiveThresholdSamplesPerAxis);
    const int regionalSamplesPerAxis = std::max(1, caveSettings.regionalEqualizationSamplesPerAxis);

    Noise2DSettings caveMain{
        .frequency = 0.014 / caveSize,
        .amplitude = 1.0,
        .octaves = 4,
        .lacunarity = 2.0,
        .persistence = 0.50,
        .rotationRadians = 0.58
    };
    Noise2DSettings caveEdge{
        .frequency = 0.052 * caveSettings.edgeRoughness,
        .amplitude = 1.0,
        .octaves = 3,
        .lacunarity = 2.05,
        .persistence = 0.48,
        .rotationRadians = -0.79
    };

    auto rawCaveSignal = [&](double logicalX, double logicalY) {
        double a = PositiveNoise(FractalPerlin2D(logicalX,
                                                 logicalY,
                                                 caveMain,
                                                 3001ULL));
        double b = PositiveNoise(FractalPerlin2D(logicalX,
                                                 logicalY,
                                                 caveEdge,
                                                 3011ULL)) * (0.42 + caveSettings.edgeRoughness * 0.14);
        return a - b;
    };

    auto chunkAverageCaveSignal = [&](ChunkPos centerChunk) {
        double sum = 0.0;
        int count = 0;

        for (int cy = centerChunk.cy - adaptiveNeighborhood; cy <= centerChunk.cy + adaptiveNeighborhood; ++cy) {
            for (int cx = centerChunk.cx - adaptiveNeighborhood; cx <= centerChunk.cx + adaptiveNeighborhood; ++cx) {
                for (int sy = 0; sy < adaptiveSamplesPerAxis; ++sy) {
                    for (int sx = 0; sx < adaptiveSamplesPerAxis; ++sx) {
                        double worldX = static_cast<double>(cx * Constants::CHUNK_SIZE)
                                      + (static_cast<double>(sx) + 0.5)
                                      * static_cast<double>(Constants::CHUNK_SIZE)
                                      / static_cast<double>(adaptiveSamplesPerAxis);
                        double worldY = static_cast<double>(cy * Constants::CHUNK_SIZE)
                                      + (static_cast<double>(sy) + 0.5)
                                      * static_cast<double>(Constants::CHUNK_SIZE)
                                      / static_cast<double>(adaptiveSamplesPerAxis);

                        if (worldX < 0.0 || worldX >= static_cast<double>(world.GetWidth())) continue;
                        if (worldY < 0.0 || worldY >= static_cast<double>(world.GetHeight())) continue;

                        sum += rawCaveSignal(worldX + static_cast<double>(Constants::WORLD_MIN_X),
                                             worldY + static_cast<double>(Constants::WORLD_MIN_Y));
                        ++count;
                    }
                }
            }
        }

        if (count == 0) return caveSettings.adaptiveThresholdTarget;
        return sum / static_cast<double>(count);
    };

    double adaptiveThresholdOffset[3][3]{};
    for (int oy = -1; oy <= 1; ++oy) {
        for (int ox = -1; ox <= 1; ++ox) {
            ChunkPos sampleChunk{chunkPos.cx + ox, chunkPos.cy + oy};
            double averageSignal = chunkAverageCaveSignal(sampleChunk);
            double offset = (averageSignal - caveSettings.adaptiveThresholdTarget)
                          * caveSettings.adaptiveThresholdStrength;
            adaptiveThresholdOffset[oy + 1][ox + 1] = Math::Clamp(
                offset,
                -caveSettings.adaptiveThresholdMaxOffset,
                caveSettings.adaptiveThresholdMaxOffset
            );
        }
    }

    auto adaptiveThresholdAt = [&](int worldX, int worldY) {
        double chunkX = (static_cast<double>(worldX) + 0.5)
                      / static_cast<double>(Constants::CHUNK_SIZE) - 0.5;
        double chunkY = (static_cast<double>(worldY) + 0.5)
                      / static_cast<double>(Constants::CHUNK_SIZE) - 0.5;
        int x0 = static_cast<int>(std::floor(chunkX));
        int y0 = static_cast<int>(std::floor(chunkY));
        double tx = SmoothStep(0.0, 1.0, chunkX - static_cast<double>(x0));
        double ty = SmoothStep(0.0, 1.0, chunkY - static_cast<double>(y0));

        auto offsetAt = [&](int cx, int cy) {
            int ix = Math::Clamp(cx - chunkPos.cx + 1, 0, 2);
            int iy = Math::Clamp(cy - chunkPos.cy + 1, 0, 2);
            return adaptiveThresholdOffset[iy][ix];
        };

        double top = Math::Lerp(offsetAt(x0,     y0),
                                offsetAt(x0 + 1, y0),
                                tx);
        double bottom = Math::Lerp(offsetAt(x0,     y0 + 1),
                                   offsetAt(x0 + 1, y0 + 1),
                                   tx);
        return Math::Lerp(top, bottom, ty);
    };

    auto distributionCoverage = [&](double logicalX, double logicalY) {
        int baseCellX = static_cast<int>(std::floor(logicalX / distributionCellSize));
        int baseCellY = static_cast<int>(std::floor(logicalY / distributionCellSize));
        double bestCoverage = 0.0;

        for (int cellY = baseCellY - 1; cellY <= baseCellY + 1; ++cellY) {
            for (int cellX = baseCellX - 1; cellX <= baseCellX + 1; ++cellX) {
                double anchorX = (static_cast<double>(cellX) + 0.35 + Hash01(cellX, cellY, m_seed, 4001ULL) * 0.30)
                               * static_cast<double>(distributionCellSize);
                double anchorY = (static_cast<double>(cellY) + 0.35 + Hash01(cellX, cellY, m_seed, 4011ULL) * 0.30)
                               * static_cast<double>(distributionCellSize);

                double dx = (logicalX - anchorX) / static_cast<double>(distributionCellSize);
                double dy = (logicalY - anchorY) / static_cast<double>(distributionCellSize);
                double distance = std::sqrt(dx * dx + dy * dy);
                double coverage = 1.0 - SmoothStep(anchorInnerRadius, anchorOuterRadius, distance);
                bestCoverage = std::max(bestCoverage, coverage);
            }
        }

        return bestCoverage;
    };

    auto caveScore = [&](double logicalX, double logicalY) {
        double coverage = distributionCoverage(logicalX, logicalY);
        double gatedCoverage = Math::Clamp(
            (coverage - minimumAnchorCoverage) / (1.0 - minimumAnchorCoverage),
            0.0,
            1.0
        );
        return rawCaveSignal(logicalX, logicalY) + gatedCoverage * uniformity * 0.045;
    };

    auto regionalAverageCaveScore = [&](int cellX, int cellY) {
        double sum = 0.0;
        int count = 0;

        for (int sy = 0; sy < regionalSamplesPerAxis; ++sy) {
            for (int sx = 0; sx < regionalSamplesPerAxis; ++sx) {
                double logicalX = (static_cast<double>(cellX)
                                + (static_cast<double>(sx) + 0.5)
                                / static_cast<double>(regionalSamplesPerAxis))
                                * static_cast<double>(distributionCellSize);
                double logicalY = (static_cast<double>(cellY)
                                + (static_cast<double>(sy) + 0.5)
                                / static_cast<double>(regionalSamplesPerAxis))
                                * static_cast<double>(distributionCellSize);

                double worldX = logicalX - static_cast<double>(Constants::WORLD_MIN_X);
                double worldY = logicalY - static_cast<double>(Constants::WORLD_MIN_Y);
                if (worldX < 0.0 || worldX >= static_cast<double>(world.GetWidth())) continue;
                if (worldY < 0.0 || worldY >= static_cast<double>(world.GetHeight())) continue;

                sum += caveScore(logicalX, logicalY);
                ++count;
            }
        }

        if (count == 0) return caveSettings.regionalEqualizationTarget;
        return sum / static_cast<double>(count);
    };

    int minLogicalX = ToLogicalX(origin.x);
    int maxLogicalX = ToLogicalX(origin.x + Constants::CHUNK_SIZE - 1);
    int minLogicalY = ToLogicalY(origin.y);
    int maxLogicalY = ToLogicalY(origin.y + Constants::CHUNK_SIZE - 1);
    int regionalMinX = static_cast<int>(std::floor(static_cast<double>(minLogicalX) / distributionCellSize - 0.5)) - 1;
    int regionalMaxX = static_cast<int>(std::floor(static_cast<double>(maxLogicalX) / distributionCellSize - 0.5)) + 2;
    int regionalMinY = static_cast<int>(std::floor(static_cast<double>(minLogicalY) / distributionCellSize - 0.5)) - 1;
    int regionalMaxY = static_cast<int>(std::floor(static_cast<double>(maxLogicalY) / distributionCellSize - 0.5)) + 2;
    int regionalWidth = regionalMaxX - regionalMinX + 1;
    int regionalHeight = regionalMaxY - regionalMinY + 1;
    std::vector<double> regionalThresholdOffset(static_cast<size_t>(regionalWidth * regionalHeight), 0.0);

    auto regionalIndex = [&](int cellX, int cellY) {
        return (cellY - regionalMinY) * regionalWidth + (cellX - regionalMinX);
    };

    for (int cellY = regionalMinY; cellY <= regionalMaxY; ++cellY) {
        for (int cellX = regionalMinX; cellX <= regionalMaxX; ++cellX) {
            double averageScore = regionalAverageCaveScore(cellX, cellY);
            double offset = (averageScore - caveSettings.regionalEqualizationTarget)
                          * caveSettings.regionalEqualizationStrength;
            regionalThresholdOffset[static_cast<size_t>(regionalIndex(cellX, cellY))] = Math::Clamp(
                offset,
                -caveSettings.regionalEqualizationMaxOffset,
                caveSettings.regionalEqualizationMaxOffset
            );
        }
    }

    auto regionalThresholdAt = [&](int logicalX, int logicalY) {
        double cellX = static_cast<double>(logicalX) / static_cast<double>(distributionCellSize) - 0.5;
        double cellY = static_cast<double>(logicalY) / static_cast<double>(distributionCellSize) - 0.5;
        int x0 = static_cast<int>(std::floor(cellX));
        int y0 = static_cast<int>(std::floor(cellY));
        double tx = SmoothStep(0.0, 1.0, cellX - static_cast<double>(x0));
        double ty = SmoothStep(0.0, 1.0, cellY - static_cast<double>(y0));

        auto offsetAt = [&](int cellOffsetX, int cellOffsetY) {
            int clampedX = Math::Clamp(cellOffsetX, regionalMinX, regionalMaxX);
            int clampedY = Math::Clamp(cellOffsetY, regionalMinY, regionalMaxY);
            return regionalThresholdOffset[static_cast<size_t>(regionalIndex(clampedX, clampedY))];
        };

        double top = Math::Lerp(offsetAt(x0,     y0),
                                offsetAt(x0 + 1, y0),
                                tx);
        double bottom = Math::Lerp(offsetAt(x0,     y0 + 1),
                                   offsetAt(x0 + 1, y0 + 1),
                                   tx);
        return Math::Lerp(top, bottom, ty);
    };

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        int worldY = origin.y + ly;

        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            int worldX = origin.x + lx;
            if (!world.IsInBounds({worldX, worldY})) continue;

            Tile& tile = chunk.GetTile(lx, ly);
            if (!CanCaveCarve(tile.type)) continue;

            int surfaceY = GetSurfaceHeight(worldX);
            int depth = worldY - surfaceY;
            if (depth < caveSettings.surfaceProtectionDepth) continue;

            int logicalX = ToLogicalX(worldX);
            int logicalY = ToLogicalY(worldY);

            double coverage = distributionCoverage(logicalX, logicalY);
            double gatedCoverage = Math::Clamp(
                (coverage - minimumAnchorCoverage) / (1.0 - minimumAnchorCoverage),
                0.0,
                1.0
            );
            double lowCoverage = 1.0 - Math::Clamp(coverage / minimumAnchorCoverage, 0.0, 1.0);

            double depthEase = Math::Clamp(
                static_cast<double>(depth - caveSettings.surfaceProtectionDepth) / 150.0,
                0.0,
                1.0
            );
            double densityShift = (caveSettings.density - 1.0) * 0.08;
            double sizeShift = (caveSize - 1.0) * 0.035;
            double threshold = Math::Lerp(0.46, 0.35, depthEase) - densityShift - sizeShift;
            threshold -= gatedCoverage * uniformity * 0.09;
            threshold += (1.0 - gatedCoverage) * separationStrength * 0.10;
            threshold += lowCoverage * separationStrength * 0.06;
            threshold += adaptiveThresholdAt(worldX, worldY);
            threshold += regionalThresholdAt(logicalX, logicalY);
            threshold = Math::Clamp(threshold, 0.22, 0.58);
            double caveValue = caveScore(static_cast<double>(logicalX),
                                         static_cast<double>(logicalY));

            if (caveValue > threshold) {
                tile.Clear();
            }
        }
    }
}

} // namespace TR
