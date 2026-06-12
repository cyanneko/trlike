#pragma once

#include "Chunk.hpp"
#include "Core/Types.hpp"
#include "Core/Constants.hpp"
#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>

namespace TR {

// ============================================================
// World — 世界管理器
// 负责管理所有Chunk的加载/卸载/查询/修改
// ============================================================
class World {
public:
    World();
    ~World();

    // --- 初始化 ---
    /// 创建新世界 (程序化生成)
    void CreateNew(int width, int height, uint64_t seed);

    /// 从存档加载世界 (Phase 2)
    /// bool LoadFromFile(const std::string& path);

    // --- Chunk管理 ---
    /// 确保某个Chunk已加载 (不存在则生成)
    Chunk& EnsureChunk(ChunkPos pos);
    Chunk& EnsureChunk(int cx, int cy);

    /// 卸载远离玩家的Chunk
    void UnloadDistantChunks(BlockPos playerPos);

    /// 获取Chunk (可能返回nullptr)
    [[nodiscard]] Chunk*       GetChunk(ChunkPos pos);
    [[nodiscard]] const Chunk* GetChunk(ChunkPos pos) const;

    /// 检查Chunk是否已加载
    [[nodiscard]] bool IsChunkLoaded(ChunkPos pos) const;

    // --- 瓦片访问 (世界坐标) ---
    /// 获取指定世界坐标的Tile (自动处理Chunk边界)
    [[nodiscard]] Tile*       GetTile(BlockPos pos);
    [[nodiscard]] const Tile* GetTile(BlockPos pos) const;
    [[nodiscard]] Tile*       GetTile(int x, int y);
    [[nodiscard]] const Tile* GetTile(int x, int y) const;

    /// 设置Tile (并标记Chunk为脏)
    void SetTile(BlockPos pos, Tile tile);
    void SetTile(BlockPos pos, TileID type);

    /// 安全地获取Tile，越界返回nullptr
    [[nodiscard]] Tile*       GetTileSafe(BlockPos pos);
    [[nodiscard]] const Tile* GetTileSafe(BlockPos pos) const;

    // --- 坐标转换工具 ---
    [[nodiscard]] static ChunkPos BlockToChunkPos(BlockPos bp);
    [[nodiscard]] static ChunkPos BlockToChunkPos(int x, int y);
    [[nodiscard]] static LocalPos BlockToLocalPos(BlockPos bp);
    [[nodiscard]] static BlockPos ChunkLocalToBlock(ChunkPos cp, LocalPos lp);

    // --- 查询 ---
    [[nodiscard]] int GetWidth()  const { return m_width; }
    [[nodiscard]] int GetHeight() const { return m_height; }
    [[nodiscard]] uint64_t GetSeed() const { return m_seed; }
    [[nodiscard]] size_t GetLoadedChunkCount() const { return m_chunks.size(); }
    [[nodiscard]] bool IsInBounds(BlockPos pos) const;

    // --- 遍历所有已加载的Chunk ---
    template<typename Func>
    void ForEachChunk(Func&& func) {
        for (auto& [pos, chunk] : m_chunks) {
            func(pos, chunk);
        }
    }

private:
    /// 生成一个Chunk (委托给WorldGenerator)
    void GenerateChunk(Chunk& chunk);

    int m_width  = Constants::WORLD_WIDTH;
    int m_height = Constants::WORLD_HEIGHT;
    uint64_t m_seed = 0;

    std::unordered_map<ChunkPos, Chunk, ChunkPosHash> m_chunks;

    friend class WorldGenerator;
};

} // namespace TR
