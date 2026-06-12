#include "World/World.hpp"
#include "World/WorldGenerator.hpp"
#include "Core/Constants.hpp"
#include <cassert>
#include <algorithm>

namespace TR {

// ============================================================
// World 实现
// ============================================================
World::World() = default;
World::~World() = default;

void World::CreateNew(int width, int height, uint64_t seed) {
    m_width  = width;
    m_height = height;
    m_seed   = seed;
    m_chunks.clear();

    // 创建世界生成器并生成初始地形
    WorldGenerator generator(seed);
    generator.GenerateWorld(*this);
}

Chunk& World::EnsureChunk(ChunkPos pos) {
    auto it = m_chunks.find(pos);
    if (it != m_chunks.end()) {
        return it->second;
    }
    // 创建新Chunk并生成
    auto [inserted, _] = m_chunks.emplace(pos, Chunk(pos));
    Chunk& chunk = inserted->second;
    GenerateChunk(chunk);
    return chunk;
}

Chunk& World::EnsureChunk(int cx, int cy) {
    return EnsureChunk(ChunkPos{cx, cy});
}

void World::UnloadDistantChunks(BlockPos playerPos) {
    ChunkPos playerChunk = BlockToChunkPos(playerPos);
    int radius = Constants::ACTIVE_CHUNK_RADIUS;

    auto it = m_chunks.begin();
    while (it != m_chunks.end()) {
        int dx = std::abs(it->first.cx - playerChunk.cx);
        int dy = std::abs(it->first.cy - playerChunk.cy);
        if (dx > radius || dy > radius) {
            it = m_chunks.erase(it);
        } else {
            ++it;
        }
    }
}

Chunk* World::GetChunk(ChunkPos pos) {
    auto it = m_chunks.find(pos);
    return it != m_chunks.end() ? &it->second : nullptr;
}

const Chunk* World::GetChunk(ChunkPos pos) const {
    auto it = m_chunks.find(pos);
    return it != m_chunks.end() ? &it->second : nullptr;
}

bool World::IsChunkLoaded(ChunkPos pos) const {
    return m_chunks.contains(pos);
}

Tile* World::GetTile(BlockPos pos) {
    Chunk* chunk = GetChunk(BlockToChunkPos(pos));
    if (!chunk) return nullptr;
    LocalPos lp = BlockToLocalPos(pos);
    return &chunk->GetTile(lp);
}

const Tile* World::GetTile(BlockPos pos) const {
    // const_cast 在这里是安全的，因为我们返回const指针
    return const_cast<World*>(this)->GetTile(pos);
}

Tile* World::GetTile(int x, int y) {
    return GetTile(BlockPos{x, y});
}

const Tile* World::GetTile(int x, int y) const {
    return GetTile(BlockPos{x, y});
}

void World::SetTile(BlockPos pos, Tile tile) {
    if (!IsInBounds(pos)) return;
    Chunk& chunk = EnsureChunk(BlockToChunkPos(pos));
    LocalPos lp = BlockToLocalPos(pos);
    chunk.GetTile(lp) = tile;
    chunk.MarkDirty();
}

void World::SetTile(BlockPos pos, TileID type) {
    if (!IsInBounds(pos)) return;
    Chunk& chunk = EnsureChunk(BlockToChunkPos(pos));
    LocalPos lp = BlockToLocalPos(pos);
    chunk.GetTile(lp).type = type;
    chunk.MarkDirty();
}

Tile* World::GetTileSafe(BlockPos pos) {
    if (!IsInBounds(pos)) return nullptr;
    return GetTile(pos);
}

const Tile* World::GetTileSafe(BlockPos pos) const {
    if (!IsInBounds(pos)) return nullptr;
    return GetTile(pos);
}

ChunkPos World::BlockToChunkPos(BlockPos bp) {
    return ChunkPos::FromBlock(bp, Constants::CHUNK_SIZE);
}

ChunkPos World::BlockToChunkPos(int x, int y) {
    return ChunkPos::FromBlock(x, y, Constants::CHUNK_SIZE);
}

LocalPos World::BlockToLocalPos(BlockPos bp) {
    return LocalPos::FromBlock(bp.x, bp.y, Constants::CHUNK_SIZE);
}

BlockPos World::ChunkLocalToBlock(ChunkPos cp, LocalPos lp) {
    return { cp.cx * Constants::CHUNK_SIZE + lp.lx,
             cp.cy * Constants::CHUNK_SIZE + lp.ly };
}

bool World::IsInBounds(BlockPos pos) const {
    return pos.x >= 0 && pos.x < m_width &&
           pos.y >= 0 && pos.y < m_height;
}

void World::GenerateChunk(Chunk& chunk) {
    WorldGenerator generator(m_seed);
    generator.GenerateChunk(*this, chunk);
}

} // namespace TR
