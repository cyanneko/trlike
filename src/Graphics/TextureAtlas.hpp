#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Rect.hpp>
#include "Core/Types.hpp"
#include <unordered_map>

namespace TR {

// ============================================================
// TextureAtlas — 管理瓦片贴图图集
// 程序化生成每个Tile/Wall的纹理，按网格排列在单张纹理上
// ============================================================
class TextureAtlas {
public:
    TextureAtlas();

    /// 程序化生成图集纹理 (不需要外部图片文件)
    void Generate();

    /// 从文件加载图集 (备选方案)
    bool LoadFromFile(const std::string& path);

    /// 获取 SFML 纹理
    [[nodiscard]] const sf::Texture& GetTexture() const { return m_texture; }

    /// 获取指定 TileID 的纹理矩形 (在图集中的位置)
    [[nodiscard]] sf::FloatRect GetTileTexRect(TileID id, uint8_t frameVariant = 0) const;

    /// 获取指定 WallID 的纹理矩形
    [[nodiscard]] sf::FloatRect GetWallTexRect(WallID id) const;

    /// 图集每格大小 (像素)
    [[nodiscard]] int GetTileSize() const { return m_tileTexSize; }

private:
    /// 绘制单个瓦片像素图样
    void DrawTileToImage(sf::Image& img, int col, int row,
                         uint8_t r, uint8_t g, uint8_t b);

    /// 生成泥土纹理
    void DrawDirt(sf::Image& img, int col, int row);
    /// 生成石头纹理
    void DrawStone(sf::Image& img, int col, int row);
    /// 生成草方块纹理
    void DrawGrass(sf::Image& img, int col, int row);
    /// 生成矿物纹理
    void DrawOre(sf::Image& img, int col, int row,
                 uint8_t r, uint8_t g, uint8_t b);
    /// 生成背景墙纹理 (深色)
    void DrawWall(sf::Image& img, int col, int row,
                  uint8_t r, uint8_t g, uint8_t b);

    sf::Texture m_texture;
    int m_tileTexSize = 16;       // 每个瓦片纹理 16×16 像素
    int m_cols = 8;               // 图集列数
    int m_rows = 4;               // 图集行数

    // TileID → (列, 行) 映射
    std::unordered_map<TileID, std::pair<int, int>> m_tilePositions;
    // WallID → (列, 行) 映射
    std::unordered_map<WallID, std::pair<int, int>> m_wallPositions;
};

} // namespace TR
