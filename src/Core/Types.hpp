#pragma once

#include <cstdint>
#include <functional>
#include <compare>

namespace TR {

// ============================================================
// 瓦片类型ID
// ============================================================
using TileID = uint16_t;
using WallID = uint16_t;

constexpr TileID TILE_AIR   = 0;
constexpr TileID TILE_DIRT  = 1;
constexpr TileID TILE_STONE = 2;
constexpr TileID TILE_GRASS = 3;
constexpr TileID TILE_SAND  = 4;
constexpr TileID TILE_CLAY  = 5;
constexpr TileID TILE_WOOD  = 6;
constexpr TileID TILE_ORE_COPPER  = 10;
constexpr TileID TILE_ORE_IRON    = 11;
constexpr TileID TILE_ORE_SILVER  = 12;
constexpr TileID TILE_ORE_GOLD    = 13;
constexpr TileID TILE_MAX   = UINT16_MAX;

constexpr WallID WALL_NONE     = 0;
constexpr WallID WALL_WOOD     = 1;
constexpr WallID WALL_STONE    = 2;
constexpr WallID WALL_DIRT     = 3;

// ============================================================
// 坐标类型
// ============================================================

/// 世界空间的瓦片坐标 (绝对坐标)
struct BlockPos {
    int x = 0;
    int y = 0;

    auto operator<=>(const BlockPos&) const = default;

    BlockPos operator+(const BlockPos& other) const { return {x + other.x, y + other.y}; }
    BlockPos operator-(const BlockPos& other) const { return {x - other.x, y - other.y}; }
    BlockPos& operator+=(const BlockPos& other) { x += other.x; y += other.y; return *this; }
    BlockPos& operator-=(const BlockPos& other) { x -= other.x; y -= other.y; return *this; }
};

/// 区块坐标
struct ChunkPos {
    int cx = 0;
    int cy = 0;

    auto operator<=>(const ChunkPos&) const = default;

    ChunkPos operator+(const ChunkPos& other) const { return {cx + other.cx, cy + other.cy}; }

    /// 从BlockPos计算对应的ChunkPos (处理负坐标)
    static ChunkPos FromBlock(int blockX, int blockY, int chunkSize) {
        return {
            blockX >= 0 ? blockX / chunkSize : (blockX + 1) / chunkSize - 1,
            blockY >= 0 ? blockY / chunkSize : (blockY + 1) / chunkSize - 1
        };
    }
    static ChunkPos FromBlock(BlockPos bp, int chunkSize) {
        return FromBlock(bp.x, bp.y, chunkSize);
    }

    /// 获取该Chunk左上角的BlockPos
    BlockPos ToBlockPos(int chunkSize) const {
        return { cx * chunkSize, cy * chunkSize };
    }
};

/// 区块内局部坐标 (0 ~ chunkSize-1)
struct LocalPos {
    int lx = 0;
    int ly = 0;

    auto operator<=>(const LocalPos&) const = default;

    /// 从BlockPos计算局部坐标 (总是返回非负值)
    static LocalPos FromBlock(int blockX, int blockY, int chunkSize) {
        int lx = blockX % chunkSize;
        int ly = blockY % chunkSize;
        if (lx < 0) lx += chunkSize;
        if (ly < 0) ly += chunkSize;
        return { lx, ly };
    }
};

// ============================================================
// 方向
// ============================================================
enum class Direction : uint8_t {
    Up    = 0,
    Down  = 1,
    Left  = 2,
    Right = 3,
    None  = 4,
};

// ============================================================
// 液体
// ============================================================
enum class LiquidType : uint8_t {
    None  = 0,
    Water = 1,
    Lava  = 2,
    Honey = 3,
};

// ============================================================
// Tile标志位
// ============================================================
namespace TileFlags {
    constexpr uint8_t HAS_ACTUATOR = 1 << 0;  // 虚化器
    constexpr uint8_t RED_WIRE     = 1 << 1;  // 红线
    constexpr uint8_t BLUE_WIRE    = 1 << 2;  // 蓝线
    constexpr uint8_t GREEN_WIRE   = 1 << 3;  // 绿线
    constexpr uint8_t YELLOW_WIRE  = 1 << 4;  // 黄线
    constexpr uint8_t INACTIVE     = 1 << 5;  // 非活跃（虚化状态）
}

// ============================================================
// 哈希支持 (用于 std::unordered_map)
// ============================================================
struct ChunkPosHash {
    size_t operator()(const ChunkPos& cp) const {
        // 将两个 int 合并为一个 size_t
        return std::hash<uint64_t>{}(
            (static_cast<uint64_t>(static_cast<uint32_t>(cp.cx)) << 32) |
            static_cast<uint64_t>(static_cast<uint32_t>(cp.cy))
        );
    }
};

} // namespace TR
