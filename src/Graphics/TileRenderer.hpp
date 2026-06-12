#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "World/World.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/TextureAtlas.hpp"
#include <unordered_map>

namespace TR {

// ============================================================
// TileRenderer — 从Chunk构建纹理化渲染网格并用相机裁剪渲染
// ============================================================
class TileRenderer {
public:
    explicit TileRenderer(const TextureAtlas& atlas);

    /// 渲染世界（确保可见Chunk加载 + 重建脏网格 + 绘制）
    void Render(World& world, const Camera& camera, sf::RenderTarget& target);

private:
    /// 为一个Chunk构建带纹理坐标的 VertexArray
    void BuildChunkMesh(const Chunk& chunk);

    const TextureAtlas& m_atlas;

    std::unordered_map<ChunkPos, sf::VertexArray, ChunkPosHash> m_meshCache;
};

} // namespace TR
