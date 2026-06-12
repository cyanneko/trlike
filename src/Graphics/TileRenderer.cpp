#include "Graphics/TileRenderer.hpp"
#include "Core/Constants.hpp"
#include <cmath>
#include <vector>

namespace TR {

TileRenderer::TileRenderer(const TextureAtlas& atlas)
    : m_atlas(atlas)
{
}

// ============================================================
// 构建 Chunk 渲染网格（带纹理坐标）
// ============================================================
void TileRenderer::BuildChunkMesh(const Chunk& chunk) {
    constexpr float TS = static_cast<float>(Constants::TILE_SIZE);
    BlockPos origin = chunk.GetWorldOrigin();
    float ox = static_cast<float>(origin.x) * TS;
    float oy = static_cast<float>(origin.y) * TS;

    std::vector<sf::Vertex> wallVertices;
    std::vector<sf::Vertex> tileVertices;

    for (int ly = 0; ly < Constants::CHUNK_SIZE; ++ly) {
        for (int lx = 0; lx < Constants::CHUNK_SIZE; ++lx) {
            const Tile& tile = chunk.GetTile(lx, ly);
            float px = ox + lx * TS;
            float py = oy + ly * TS;

            // === 背景墙 ===
            if (tile.HasWall()) {
                sf::FloatRect tr = m_atlas.GetWallTexRect(tile.wall);
                wallVertices.push_back(sf::Vertex({px, py},           {tr.left, tr.top}));
                wallVertices.push_back(sf::Vertex({px + TS, py},      {tr.left + tr.width, tr.top}));
                wallVertices.push_back(sf::Vertex({px + TS, py + TS}, {tr.left + tr.width, tr.top + tr.height}));
                wallVertices.push_back(sf::Vertex({px, py + TS},      {tr.left, tr.top + tr.height}));
            }

            // === 实体瓦片 ===
            if (tile.IsSolid()) {
                sf::FloatRect tr = m_atlas.GetTileTexRect(tile.type, tile.frameVariant);

                // 下半部分带一点阴影（立体感）
                sf::Color topColor = sf::Color::White;
                sf::Color sideColor(220, 220, 220);  // 略暗

                tileVertices.push_back(sf::Vertex({px, py},           topColor,  {tr.left, tr.top}));
                tileVertices.push_back(sf::Vertex({px + TS, py},      topColor,  {tr.left + tr.width, tr.top}));
                tileVertices.push_back(sf::Vertex({px + TS, py + TS}, sideColor,{tr.left + tr.width, tr.top + tr.height}));
                tileVertices.push_back(sf::Vertex({px, py + TS},      sideColor,{tr.left, tr.top + tr.height}));
            }
        }
    }

    // 合并（墙 + 瓦片）
    sf::VertexArray& va = m_meshCache[chunk.GetPosition()];
    va.setPrimitiveType(sf::Quads);
    va.clear();
    for (const auto& v : wallVertices) va.append(v);
    for (const auto& v : tileVertices) va.append(v);
}

// ============================================================
// 渲染
// ============================================================
void TileRenderer::Render(World& world, const Camera& /*camera*/, sf::RenderTarget& target) {
    sf::Vector2f viewSize = target.getView().getSize();
    sf::Vector2f viewCenter = target.getView().getCenter();
    constexpr float TS = static_cast<float>(Constants::TILE_SIZE);

    float visibleTilesX = viewSize.x / TS;
    float visibleTilesY = viewSize.y / TS;
    float halfW = visibleTilesX * 0.5f + static_cast<float>(Constants::CHUNK_SIZE);
    float halfH = visibleTilesY * 0.5f + static_cast<float>(Constants::CHUNK_SIZE);

    int minBlockX = static_cast<int>(std::floor(viewCenter.x / TS - halfW));
    int maxBlockX = static_cast<int>(std::ceil(viewCenter.x / TS + halfW));
    int minBlockY = static_cast<int>(std::floor(viewCenter.y / TS - halfH));
    int maxBlockY = static_cast<int>(std::ceil(viewCenter.y / TS + halfH));

    ChunkPos minCP = ChunkPos::FromBlock(minBlockX, minBlockY, Constants::CHUNK_SIZE);
    ChunkPos maxCP = ChunkPos::FromBlock(maxBlockX, maxBlockY, Constants::CHUNK_SIZE);

    // --- 确保可见 Chunk 已加载 ---
    for (int cy = minCP.cy; cy <= maxCP.cy; ++cy) {
        for (int cx = minCP.cx; cx <= maxCP.cx; ++cx) {
            ChunkPos cp{cx, cy};
            BlockPos origin = cp.ToBlockPos(Constants::CHUNK_SIZE);
            if (origin.x >= world.GetWidth() || origin.y >= world.GetHeight()) continue;
            if (origin.x + Constants::CHUNK_SIZE < 0 || origin.y + Constants::CHUNK_SIZE < 0) continue;
            world.EnsureChunk(cp);
        }
    }

    // --- 重建脏 Chunk 网格 ---
    world.ForEachChunk([&](ChunkPos /*cp*/, Chunk& chunk) {
        ChunkPos pos = chunk.GetPosition();
        if (chunk.IsDirty() || !m_meshCache.contains(pos)) {
            BuildChunkMesh(chunk);
            chunk.MarkClean();
        }
    });

    // --- 绑定纹理并绘制 ---
    sf::RenderStates states;
    states.texture = &m_atlas.GetTexture();

    for (int cy = minCP.cy; cy <= maxCP.cy; ++cy) {
        for (int cx = minCP.cx; cx <= maxCP.cx; ++cx) {
            ChunkPos cp{cx, cy};
            auto it = m_meshCache.find(cp);
            if (it != m_meshCache.end() && it->second.getVertexCount() > 0) {
                target.draw(it->second, states);
            }
        }
    }
}

} // namespace TR
