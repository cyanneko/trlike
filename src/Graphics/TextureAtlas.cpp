#include "Graphics/TextureAtlas.hpp"
#include "Core/Math.hpp"
#include <algorithm>

namespace TR {

TextureAtlas::TextureAtlas() = default;

void TextureAtlas::Generate() {
    m_tileTexSize = 6;
    m_cols = 8;
    m_rows = 4;

    sf::Image img;
    img.create(m_cols * m_tileTexSize, m_rows * m_tileTexSize, sf::Color::Transparent);

    m_tilePositions.clear();
    m_wallPositions.clear();

    int c = 0, r = 0;
    auto nextSlot = [&]() -> std::pair<int,int> {
        auto pos = std::make_pair(c, r);
        c++; if (c >= m_cols) { c = 0; r++; }
        return pos;
    };

    // 基础方块
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_DIRT]  = {col,row}; DrawDirt(img, col, row); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_STONE] = {col,row}; DrawStone(img, col, row); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_GRASS] = {col,row}; DrawGrass(img, col, row); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_SAND]  = {col,row}; DrawTileToImage(img, col, row, 220, 190, 140); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_CLAY]  = {col,row}; DrawTileToImage(img, col, row, 170, 95, 70); }
    {
        auto [col, row] = nextSlot(); m_tilePositions[TILE_WOOD] = {col,row};
        int ox = col * m_tileTexSize, oy = row * m_tileTexSize;
        for (int y = 0; y < m_tileTexSize; ++y) {
            for (int x = 0; x < m_tileTexSize; ++x) {
                bool isLine = (y % 3 == 0);
                auto shade = static_cast<uint8_t>(isLine ? 95 : 115);
                img.setPixel(ox + x, oy + y, sf::Color(shade, static_cast<uint8_t>(shade * 0.52f), static_cast<uint8_t>(shade * 0.22f)));
            }
        }
    }

    // 矿物
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_ORE_COPPER] = {col,row}; DrawOre(img, col, row, 200, 120, 40); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_ORE_IRON]   = {col,row}; DrawOre(img, col, row, 180, 170, 160); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_ORE_SILVER] = {col,row}; DrawOre(img, col, row, 210, 210, 215); }
    { auto [col, row] = nextSlot(); m_tilePositions[TILE_ORE_GOLD]   = {col,row}; DrawOre(img, col, row, 230, 190, 50); }

    // 背景墙
    { auto [col, row] = nextSlot(); m_wallPositions[WALL_DIRT]  = {col,row}; DrawWall(img, col, row, 70, 45, 20); }
    { auto [col, row] = nextSlot(); m_wallPositions[WALL_STONE] = {col,row}; DrawWall(img, col, row, 75, 75, 78); }
    { auto [col, row] = nextSlot(); m_wallPositions[WALL_WOOD]  = {col,row}; DrawWall(img, col, row, 60, 35, 15); }

    m_texture.loadFromImage(img);
    m_texture.setSmooth(false);
}

// ============================================================
// 通用瓦片绘制
// ============================================================
void TextureAtlas::DrawTileToImage(sf::Image& img, int col, int row,
                                   uint8_t r, uint8_t g, uint8_t b) {
    int ox = col * m_tileTexSize;
    int oy = row * m_tileTexSize;

    for (int y = 0; y < m_tileTexSize; ++y) {
        for (int x = 0; x < m_tileTexSize; ++x) {
            float edge = 1.0f;
            if (x == 0 || y == m_tileTexSize - 1) edge = 0.75f;
            else if (x <= 1 || y >= m_tileTexSize - 3) edge = 0.85f;

            int hash = (x * 31 + y * 37 + col * 53 + row * 97) % 100;
            float noise = 1.0f + (hash < 15 ? (hash < 8 ? 0.08f : -0.08f) : 0.0f);

            auto cr = static_cast<uint8_t>(Math::Clamp(static_cast<int>(r * edge * noise), 0, 255));
            auto cg = static_cast<uint8_t>(Math::Clamp(static_cast<int>(g * edge * noise), 0, 255));
            auto cb = static_cast<uint8_t>(Math::Clamp(static_cast<int>(b * edge * noise), 0, 255));
            img.setPixel(ox + x, oy + y, sf::Color(cr, cg, cb));
        }
    }
}

// ============================================================
// 特定瓦片
// ============================================================
void TextureAtlas::DrawDirt(sf::Image& img, int col, int row) {
    DrawTileToImage(img, col, row, 130, 85, 40);
}

void TextureAtlas::DrawStone(sf::Image& img, int col, int row) {
    int ox = col * m_tileTexSize;
    int oy = row * m_tileTexSize;

    for (int y = 0; y < m_tileTexSize; ++y) {
        for (int x = 0; x < m_tileTexSize; ++x) {
            float edge = 1.0f;
            if (x == 0 || y == m_tileTexSize - 1) edge = 0.75f;

            int hash = (x * 41 + y * 67 + col * 101 + row * 151) % 100;
            float shade = 0.85f + (hash < 20 ? 0.15f : 0.0f) + (hash < 5 ? 0.08f : 0.0f);

            auto gray = static_cast<uint8_t>(Math::Clamp(static_cast<int>(140.0f * edge * shade), 0, 255));
            img.setPixel(ox + x, oy + y, sf::Color(gray, gray, gray));
        }
    }
}

void TextureAtlas::DrawGrass(sf::Image& img, int col, int row) {
    int ox = col * m_tileTexSize;
    int oy = row * m_tileTexSize;

    for (int y = 0; y < m_tileTexSize; ++y) {
        for (int x = 0; x < m_tileTexSize; ++x) {
            if (y < m_tileTexSize / 3) {
                float shade = (x == 0 || x == 15) ? 0.8f : 1.0f;
                img.setPixel(ox + x, oy + y,
                    sf::Color(static_cast<uint8_t>(50*shade), static_cast<uint8_t>(150*shade), static_cast<uint8_t>(30*shade)));
            } else {
                float shade = (y == 15) ? 0.75f : 1.0f;
                img.setPixel(ox + x, oy + y,
                    sf::Color(static_cast<uint8_t>(130*shade), static_cast<uint8_t>(85*shade), static_cast<uint8_t>(40*shade)));
            }
        }
    }

    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < m_tileTexSize; ++x)
            if ((x * 17 + y * 31 + col * 53) % 10 < 3)
                img.setPixel(ox + x, oy + y, sf::Color(80, 170, 40));
}

// ============================================================
// 矿物
// ============================================================
void TextureAtlas::DrawOre(sf::Image& img, int col, int row,
                           uint8_t r, uint8_t g, uint8_t b) {
    int ox = col * m_tileTexSize;
    int oy = row * m_tileTexSize;

    for (int y = 0; y < m_tileTexSize; ++y) {
        for (int x = 0; x < m_tileTexSize; ++x) {
            int hash = (x * 41 + y * 67 + col * 101 + row * 151) % 100;

            if (hash < 18) {
                float bright = (hash < 5) ? 1.2f : 1.0f;
                auto cr = static_cast<uint8_t>(Math::Clamp(static_cast<int>(r * bright), 0, 255));
                auto cg = static_cast<uint8_t>(Math::Clamp(static_cast<int>(g * bright), 0, 255));
                auto cb = static_cast<uint8_t>(Math::Clamp(static_cast<int>(b * bright), 0, 255));
                img.setPixel(ox + x, oy + y, sf::Color(cr, cg, cb));
            } else {
                float shade = 0.85f + (hash < 30 ? 0.15f : 0.0f);
                auto gray = static_cast<uint8_t>(Math::Clamp(static_cast<int>(140.0f * shade), 0, 255));
                img.setPixel(ox + x, oy + y, sf::Color(gray, gray, gray));
            }
        }
    }
}

// ============================================================
// 背景墙
// ============================================================
void TextureAtlas::DrawWall(sf::Image& img, int col, int row,
                            uint8_t r, uint8_t g, uint8_t b) {
    int ox = col * m_tileTexSize;
    int oy = row * m_tileTexSize;

    for (int y = 0; y < m_tileTexSize; ++y) {
        for (int x = 0; x < m_tileTexSize; ++x) {
            float shade = 0.5f;
            if ((x * 13 + y * 23 + col * 47) % 20 < 3) shade += 0.1f;
            img.setPixel(ox + x, oy + y,
                sf::Color(static_cast<uint8_t>(r*shade), static_cast<uint8_t>(g*shade), static_cast<uint8_t>(b*shade), 200));
        }
    }
}

// ============================================================
// 纹理矩形查询
// ============================================================
sf::FloatRect TextureAtlas::GetTileTexRect(TileID id, uint8_t /*frameVariant*/) const {
    auto it = m_tilePositions.find(id);
    if (it != m_tilePositions.end()) {
        float u = static_cast<float>(it->second.first  * m_tileTexSize);
        float v = static_cast<float>(it->second.second * m_tileTexSize);
        return {u, v, static_cast<float>(m_tileTexSize), static_cast<float>(m_tileTexSize)};
    }
    return {0.f, 0.f, static_cast<float>(m_tileTexSize), static_cast<float>(m_tileTexSize)};
}

sf::FloatRect TextureAtlas::GetWallTexRect(WallID id) const {
    auto it = m_wallPositions.find(id);
    if (it != m_wallPositions.end()) {
        float u = static_cast<float>(it->second.first  * m_tileTexSize);
        float v = static_cast<float>(it->second.second * m_tileTexSize);
        return {u, v, static_cast<float>(m_tileTexSize), static_cast<float>(m_tileTexSize)};
    }
    return {0.f, 0.f, static_cast<float>(m_tileTexSize), static_cast<float>(m_tileTexSize)};
}

bool TextureAtlas::LoadFromFile(const std::string& path) {
    return m_texture.loadFromFile(path);
}

} // namespace TR
