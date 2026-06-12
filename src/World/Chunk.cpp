#include "World/Chunk.hpp"
#include "Core/Constants.hpp"

namespace TR {

BlockPos Chunk::GetWorldOrigin() const {
    return {
        m_position.cx * Constants::CHUNK_SIZE,
        m_position.cy * Constants::CHUNK_SIZE
    };
}

Tile& Chunk::GetTile(int lx, int ly) {
    return m_tiles[TileIndex(lx, ly)];
}

const Tile& Chunk::GetTile(int lx, int ly) const {
    return m_tiles[TileIndex(lx, ly)];
}

Tile& Chunk::GetTile(LocalPos lp) {
    return GetTile(lp.lx, lp.ly);
}

const Tile& Chunk::GetTile(LocalPos lp) const {
    return GetTile(lp.lx, lp.ly);
}

void Chunk::Fill(TileID type) {
    for (auto& tile : m_tiles) {
        tile.type = type;
    }
    MarkDirty();
}

int Chunk::TileIndex(int lx, int ly) {
    return ly * Constants::CHUNK_SIZE + lx;
}

bool Chunk::IsEmpty() const {
    for (const auto& tile : m_tiles) {
        if (!tile.IsAir()) return false;
    }
    return true;
}

int Chunk::SolidTileCount() const {
    int count = 0;
    for (const auto& tile : m_tiles) {
        if (tile.IsSolid()) ++count;
    }
    return count;
}

} // namespace TR
