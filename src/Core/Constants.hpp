#pragma once

#include <cstdint>

namespace TR::Constants {

// ============================================================
// 世界常量
// ============================================================
constexpr int CHUNK_SIZE          = 32;      // 每个Chunk的边长 (32x32 tiles)
constexpr int CHUNK_TILE_COUNT    = CHUNK_SIZE * CHUNK_SIZE;  // 1024
constexpr int WORLD_MIN_X         = -2000;   // 逻辑世界左边界
constexpr int WORLD_MAX_X         = 2000;    // 逻辑世界右边界
constexpr int WORLD_MIN_Y         = -700;    // 逻辑世界上边界
constexpr int WORLD_MAX_Y         = 300;     // 逻辑世界下边界
constexpr int WORLD_WIDTH         = WORLD_MAX_X - WORLD_MIN_X + 1;
constexpr int WORLD_HEIGHT        = WORLD_MAX_Y - WORLD_MIN_Y + 1;
constexpr int WORLD_SURFACE_Y     = -WORLD_MIN_Y; // 逻辑 y=0 对应的数组Y坐标
constexpr int WORLD_CHUNKS_X      = (WORLD_WIDTH + CHUNK_SIZE - 1) / CHUNK_SIZE;
constexpr int WORLD_CHUNKS_Y      = (WORLD_HEIGHT + CHUNK_SIZE - 1) / CHUNK_SIZE;

// ============================================================
// 区块加载
// ============================================================
constexpr int ACTIVE_CHUNK_RADIUS = 4;       // 玩家周围加载的Chunk半径
constexpr int ACTIVE_CHUNK_DIAMETER = ACTIVE_CHUNK_RADIUS * 2 + 1;

// ============================================================
// 物理常量
// ============================================================
constexpr float GRAVITY           = 0.4f;    // 每帧重力加速度
constexpr float TERMINAL_VELOCITY = 16.0f;   // 最大下落速度
constexpr float PLAYER_SPEED      = 3.0f;    // 玩家移动速度
constexpr float PLAYER_JUMP_SPEED = -8.0f;   // 玩家跳跃速度
constexpr float TILE_SIZE         = 16.0f;   // 每个Tile的像素大小

// ============================================================
// 世界生成
// ============================================================
constexpr int   CAVE_DENSITY      = 45;      // 洞穴密度 (百分比)
constexpr float TERRAIN_SCALE     = 0.005f;  // 地形噪声缩放
constexpr float CAVE_SCALE        = 0.03f;   // 洞穴噪声缩放
constexpr float ORE_SCALE         = 0.08f;   // 矿物噪声缩放
constexpr int   DIRT_LAYER_DEPTH  = 15;      // 泥土层深度
constexpr int   STONE_START_DEPTH = 5;       // 石头起始深度 (从地表往下)

// ============================================================
// 游戏循环
// ============================================================
constexpr int   TARGET_FPS        = 60;
constexpr float FIXED_DT          = 1.0f / TARGET_FPS;

} // namespace TR::Constants
