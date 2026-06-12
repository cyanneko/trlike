#pragma once

#include "Core/Types.hpp"
#include <cstdint>

namespace TR {

// ============================================================
// Tile — 单个瓦片数据 (紧凑存储, 7 bytes)
// ============================================================
#pragma pack(push, 1)
struct Tile {
    TileID  type   = TILE_AIR;    // 瓦片类型 (0=空气)
    WallID  wall   = WALL_NONE;   // 背景墙类型 (0=无)
    uint8_t liquid = 0;           // 高2位:液体类型, 低6位:液体量(0-63)
    uint8_t flags  = 0;           // 标志位 (电线/虚化器等)
    uint8_t frameVariant = 0;     // 帧变种 (视觉多样性)

    // --- 便捷方法 ---

    [[nodiscard]] bool IsSolid() const { return type != TILE_AIR; }
    [[nodiscard]] bool IsAir()   const { return type == TILE_AIR; }
    [[nodiscard]] bool HasWall() const { return wall != WALL_NONE; }

    // 液体相关
    [[nodiscard]] LiquidType GetLiquidType() const {
        return static_cast<LiquidType>(liquid >> 6);
    }
    void SetLiquidType(LiquidType lt) {
        liquid = (liquid & 0x3F) | (static_cast<uint8_t>(lt) << 6);
    }
    [[nodiscard]] uint8_t GetLiquidAmount() const {
        return liquid & 0x3F;  // 0-63
    }
    void SetLiquidAmount(uint8_t amount) {
        liquid = (liquid & 0xC0) | (amount & 0x3F);
    }

    // 标志位
    [[nodiscard]] bool HasFlag(uint8_t flag) const { return (flags & flag) != 0; }
    void SetFlag(uint8_t flag)   { flags |= flag; }
    void ClearFlag(uint8_t flag) { flags &= ~flag; }

    /// 重置为空气
    void Clear() {
        type = TILE_AIR;
        wall = WALL_NONE;
        liquid = 0;
        flags = 0;
        frameVariant = 0;
    }
};
#pragma pack(pop)

static_assert(sizeof(Tile) == 7, "Tile must be 7 bytes for compact storage");

} // namespace TR
