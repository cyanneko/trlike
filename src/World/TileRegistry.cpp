#include "World/TileRegistry.hpp"
#include <cassert>

namespace TR {

// 空气的默认属性 (静态)
const TileProperties TileRegistry::AIR_PROPERTIES = {
    .id          = TILE_AIR,
    .name        = "Air",
    .solid       = false,
    .opaque      = false,
    .mergeWith   = false,
    .hardness    = 0.0f,
    .pickaxePower = 0,
};

TileRegistry& TileRegistry::Instance() {
    static TileRegistry registry;
    return registry;
}

TileRegistry::TileRegistry() {
    RegisterDefaults();
}

void TileRegistry::RegisterDefaults() {
    Register({ TILE_DIRT,  "Dirt Block",   true, true, true, 1.0f, 0 });
    Register({ TILE_STONE, "Stone Block",  true, true, true, 2.0f, 0 });
    Register({ TILE_GRASS, "Grass Block",  true, true, true, 1.0f, 0 });
    Register({ TILE_SAND,  "Sand Block",   true, true, false, 0.8f, 0 });
    Register({ TILE_CLAY,  "Clay Block",   true, true, true, 1.2f, 0 });
    Register({ TILE_WOOD,  "Wood Block",   true, true, true, 1.0f, 0 });

    // 矿物
    Register({ TILE_ORE_COPPER, "Copper Ore",  true, true, true, 2.0f, 0 });
    Register({ TILE_ORE_IRON,   "Iron Ore",    true, true, true, 2.5f, 1 });
    Register({ TILE_ORE_SILVER, "Silver Ore",  true, true, true, 3.0f, 1 });
    Register({ TILE_ORE_GOLD,   "Gold Ore",    true, true, true, 3.5f, 2 });
}

void TileRegistry::Register(TileProperties props) {
    TileID id = props.id;

    // 确保数组足够大
    if (id >= m_properties.size()) {
        m_properties.resize(id + 1);
    }

    // 注册名称映射
    m_nameMap[props.name] = id;

    // 存储属性
    m_properties[id] = std::move(props);
}

const TileProperties* TileRegistry::Get(TileID id) const {
    if (id < m_properties.size() && id != TILE_AIR) {
        return &m_properties[id];
    }
    if (id == TILE_AIR) {
        return &AIR_PROPERTIES;
    }
    return nullptr;
}

const TileProperties& TileRegistry::GetOrAir(TileID id) const {
    const TileProperties* props = Get(id);
    return props ? *props : AIR_PROPERTIES;
}

TileID TileRegistry::FindByName(std::string_view name) const {
    auto it = m_nameMap.find(std::string(name));
    return it != m_nameMap.end() ? it->second : TILE_AIR;
}

bool TileRegistry::IsSolid(TileID id) const {
    return GetOrAir(id).solid;
}

} // namespace TR
