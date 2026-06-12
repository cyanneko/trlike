#pragma once

#include "Tile.hpp"
#include "Core/Types.hpp"
#include "Core/Constants.hpp"
#include <array>
#include <memory>

namespace TR {

// ============================================================
// Chunk — 32×32 的瓦片区域，世界的基本加载单元
// ============================================================
class Chunk {
public:
    Chunk() = default;
    explicit Chunk(ChunkPos pos) : m_position(pos) {}

    // --- 坐标 ---
    [[nodiscard]] ChunkPos GetPosition() const { return m_position; }

    /// 该Chunk左上角在世界中的Block坐标
    [[nodiscard]] BlockPos GetWorldOrigin() const;

    // --- 瓦片访问 ---
    /// 通过局部坐标访问 (0~CHUNK_SIZE-1)
    [[nodiscard]] Tile&       GetTile(int lx, int ly);
    [[nodiscard]] const Tile& GetTile(int lx, int ly) const;

    /// 通过局部坐标结构体
    [[nodiscard]] Tile&       GetTile(LocalPos lp);
    [[nodiscard]] const Tile& GetTile(LocalPos lp) const;

    /// 填充整个Chunk为指定瓦片
    void Fill(TileID type);

    /// 获取局部索引转为一维数组索引
    [[nodiscard]] static int TileIndex(int lx, int ly);

    // --- 脏标记 (渲染优化) ---
    /// 标记此Chunk需要重建渲染网格
    void MarkDirty() { m_isDirty = true; }
    void MarkClean() { m_isDirty = false; }
    [[nodiscard]] bool IsDirty() const { return m_isDirty; }

    // --- 查询 ---
    /// 此Chunk是否完全为空 (全是空气)
    [[nodiscard]] bool IsEmpty() const;

    /// 获取非空气Tile的数量
    [[nodiscard]] int SolidTileCount() const;

private:
    ChunkPos m_position;
    std::array<Tile, Constants::CHUNK_TILE_COUNT> m_tiles{};  // 默认初始化为空气
    bool m_isDirty = true;  // 新Chunk默认为脏，需要生成/加载
};

} // namespace TR
