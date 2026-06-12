#pragma once

#include "Core/Types.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace TR {

// ============================================================
// TileProperties — 瓦片类型的属性定义 (数据驱动)
// ============================================================
struct TileProperties {
    TileID      id;
    std::string name;              // 名称 (如 "Stone Block")
    bool        solid       = true;// 是否实体碰撞
    bool        opaque      = true;// 是否不透明 (影响光照)
    bool        mergeWith   = true;// 是否与同类瓦片合并渲染
    float       hardness    = 1.0f;// 硬度 (挖掘所需时间倍数)
    uint8_t     pickaxePower = 0;  // 挖掘所需镐力
    bool        canGrowGrass = false; // 能否长草
    bool        canBeCorrupted = false; // 能否被腐化
};

// ============================================================
// TileRegistry — 全局瓦片类型注册表
// ============================================================
class TileRegistry {
public:
    static TileRegistry& Instance();

    /// 注册一个瓦片类型
    void Register(TileProperties props);

    /// 查询属性
    [[nodiscard]] const TileProperties* Get(TileID id) const;
    [[nodiscard]] const TileProperties& GetOrAir(TileID id) const;

    /// 通过名称查找
    [[nodiscard]] TileID FindByName(std::string_view name) const;

    /// 是否为实心方块
    [[nodiscard]] bool IsSolid(TileID id) const;

    /// 获取注册总数
    [[nodiscard]] size_t Count() const { return m_properties.size(); }

private:
    TileRegistry();
    void RegisterDefaults();

    std::vector<TileProperties> m_properties;       // 按ID索引
    std::unordered_map<std::string, TileID> m_nameMap;

    static const TileProperties AIR_PROPERTIES;      // 空气的默认属性
};

} // namespace TR
