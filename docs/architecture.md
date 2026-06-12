# TRlike - 类Terraria游戏架构设计

## 技术选型

| 层级 | 技术 | 说明 |
|------|------|------|
| 语言 | C++20 | 现代C++，使用智能指针、concepts |
| 构建 | CMake 3.20+ | 跨平台构建系统 |
| 图形 | SFML 2.6+ | 2D渲染、窗口管理、输入、音频 |
| 数学 | 自研 / GLM | 向量、矩阵运算 |
| 序列化 | 自研二进制 / nlohmann-json | 世界存档 |
| 噪声 | FastNoiseLite | 地形生成 |

## 整体架构 (分层设计)

```
┌─────────────────────────────────────────────┐
│                  Game Layer                  │
│  Game Loop │ State Machine │ Event Bus      │
├─────────────────────────────────────────────┤
│              System Layer (ECS)              │
│  Physics │ Render │ AI │ Combat │ Liquid    │
│  Lighting │ Audio │ Weather │ Wiring       │
├─────────────────────────────────────────────┤
│              Component Layer                 │
│  Position │ Velocity │ Health │ Sprite      │
│  Inventory │ AIBrain │ Projectile │ Tile    │
├─────────────────────────────────────────────┤
│               Entity Layer                   │
│  Entity Manager │ Archetypes │ Pools        │
├─────────────────────────────────────────────┤
│               World Layer                    │
│  Chunk │ Tile │ WorldGen │ Biome │ Save     │
├─────────────────────────────────────────────┤
│              Platform Layer                  │
│  SFML │ OpenGL │ FileIO │ Network           │
└─────────────────────────────────────────────┘
```

## 核心系统设计

### 1. 世界系统 (World System)

```
┌──────────────────────────────────────────┐
│                 World                     │
│  ┌────────┐ ┌────────┐ ┌────────┐       │
│  │ Chunk  │ │ Chunk  │ │ Chunk  │ ...   │
│  │ (0,0)  │ │ (1,0)  │ │ (2,0)  │       │
│  └────────┘ └────────┘ └────────┘       │
│  ┌────────┐ ┌────────┐ ┌────────┐       │
│  │ Chunk  │ │ Chunk  │ │ Chunk  │ ...   │
│  │ (0,1)  │ │ (1,1)  │ │ (2,1)  │       │
│  └────────┘ └────────┘ └────────┘       │
│                                          │
│  每个Chunk = 32×32 个Tile                │
│  Tile = 7 bytes (紧凑存储)               │
└──────────────────────────────────────────┘
```

**设计要点：**
- **Chunk 分区**：世界被分成固定大小的 Chunk（32×32），实现按需加载/卸载
- **Tile 紧凑存储**：每个 Tile 约 7 字节，减少内存占用
- **脏标记**：只有修改过的 Chunk 才重新构建渲染网格
- **坐标系统**：
  - `BlockPos {int x, y}` — 瓦片坐标（世界空间）
  - `ChunkPos {int cx, cy}` — 区块坐标
  - `LocalPos {int lx, ly}` — 区块内局部坐标 (0~31)

**数据流：**
```
WorldGenerator → World → Chunk[] → Tile[]
                    ↓
               RenderSystem → VertexArray
                    ↓
               PhysicsSystem → Collision Queries
```

### 2. ECS 实体系统

采用**原型(Archetype)ECS**设计（类似EnTT但简化）：

```
Entity (32位句柄)
  ├── ID (24位) — 实体唯一ID
  └── Generation (8位) — 防止悬空引用

Component (纯数据)
  ├── Transform { vec2 pos, vec2 scale, float rot }
  ├── Velocity { vec2 velocity }
  ├── Sprite { TextureID tex, Rect uv }
  ├── Health { int hp, int maxHp }
  ├── Collider { vec2 size, CollisionLayer layer }
  └── ... 可按需添加

System (纯逻辑，无状态)
  ├── PhysicsSystem — 重力、碰撞
  ├── RenderSystem — 渲染排序、绘制
  ├── AISystem — 敌人行为
  ├── CombatSystem — 伤害计算
  └── ProjectileSystem — 弹道
```

### 3. 物理系统

```
碰撞检测流程：
1. 粗检测 → 空间哈希网格 (Spatial Hash)
2. 细检测 → AABB vs AABB
3. Tile碰撞 → AABB vs 瓦片网格 (仅实体周围3×3 Chunk)
4. 分离轴 → 沿X轴分离，再沿Y轴分离
```

**碰撞层：**
```cpp
enum class CollisionLayer : uint8_t {
    None     = 0,
    Player   = 1 << 0,
    Enemy    = 1 << 1,
    NPC      = 1 << 2,
    Projectile = 1 << 3,
    Item     = 1 << 4,
    Tile     = 1 << 5,
    Liquid   = 1 << 6,
};
```

### 4. 渲染系统

```
渲染管线：
1. Tile渲染 → 构建Chunk的VertexArray（静态网格）
2. Wall渲染 → 背景墙的VertexArray
3. Entity渲染 → Sprite排序（Y轴深度排序）
4. Liquid渲染 → 液体表面网格
5. Light渲染 → 光照贴图（乘法混合）
6. UI渲染 → ImGui / 自定义UI

相机系统：
- 2D正交相机
- 平滑跟随玩家
- 支持缩放
- 视口裁剪（只渲染可见Chunk）
```

### 5. 物品与背包系统

```
Item (原型模式)
  ├── ItemID (uint16_t) — 唯一类型标识
  ├── StackSize (int) — 堆叠数量
  ├── Prefix (uint8_t) — 修饰前缀
  └── CustomData — 自定义数据（如染料颜色）

Inventory
  ├── Hotbar[10] — 快捷栏
  ├── Main[40] — 主背包
  ├── Coins[4] — 钱币槽
  ├── Ammo[4] — 弹药槽
  └── Armor[3] + Accessories[7] — 装备栏
```

### 6. 世界生成

```
生成流程 (Terraria风格)：
1. 噪声生成地形轮廓 (Perlin/Simplex)
2. 放置泥土/石块层
3. 生成洞穴系统 (Cellular Automata / Perlin Worm)
4. 放置矿物矿脉
5. 生成生物群系过渡
6. 放置结构 (生命树、金字塔、地下小屋)
7. 生成液体池 (水、岩浆)
8. 放置宝箱和初始物品
```

### 7. 多人联机架构 (远期规划)

```
Client-Server 模型：
┌──────────┐     ┌──────────┐
│ Client A │     │ Client B │
└────┬─────┘     └────┬─────┘
     │   UDP (ENet)   │
     └────────┬───────┘
         ┌────┴────┐
         │  Server  │
         │ (权威)   │
         └─────────┘

同步策略：
- 世界数据 → 按Chunk同步（仅发送玩家附近的Chunk）
- 实体状态 → 服务器权威 + 客户端预测
- 物品拾取 → 服务器验证（防作弊）
```

## 目录结构

```
TRlike/
├── CMakeLists.txt
├── README.md
├── docs/
│   └── architecture.md
├── assets/
│   ├── textures/
│   │   ├── tiles/         # 瓦片贴图
│   │   ├── entities/      # 实体贴图
│   │   ├── ui/            # UI贴图
│   │   └── backgrounds/   # 背景
│   ├── sounds/            # 音效
│   └── data/
│       ├── tiles.json     # 瓦片定义
│       ├── items.json     # 物品定义
│       └── recipes.json   # 配方定义
├── src/
│   ├── main.cpp
│   ├── Core/
│   │   ├── Types.hpp      # 基础类型 (BlockPos, ChunkPos, TileID...)
│   │   ├── Constants.hpp  # 游戏常量
│   │   ├── Game.hpp/cpp   # 游戏主循环
│   │   └── Math.hpp       # 数学工具
│   ├── World/
│   │   ├── Tile.hpp       # Tile数据结构
│   │   ├── TileRegistry.hpp/cpp  # 瓦片类型注册表
│   │   ├── Chunk.hpp/cpp  # 区块
│   │   ├── World.hpp/cpp  # 世界管理器
│   │   └── WorldGenerator.hpp/cpp # 世界生成器
│   ├── ECS/
│   │   ├── Entity.hpp     # 实体句柄
│   │   ├── Component.hpp  # 组件基类
│   │   ├── Archetype.hpp  # 原型存储
│   │   └── WorldECS.hpp   # ECS世界
│   ├── Systems/
│   │   ├── PhysicsSystem.hpp/cpp
│   │   ├── RenderSystem.hpp/cpp
│   │   ├── AISystem.hpp/cpp
│   │   └── LiquidSystem.hpp/cpp
│   ├── Graphics/
│   │   ├── Renderer.hpp/cpp
│   │   ├── Camera.hpp/cpp
│   │   ├── SpriteBatch.hpp/cpp
│   │   └── TextureAtlas.hpp/cpp
│   ├── Entities/
│   │   ├── Player.hpp/cpp
│   │   ├── Enemy.hpp/cpp
│   │   └── NPC.hpp/cpp
│   ├── Items/
│   │   ├── Item.hpp
│   │   ├── Inventory.hpp/cpp
│   │   └── ItemRegistry.hpp/cpp
│   └── UI/
│       ├── UIManager.hpp/cpp
│       └── widgets/
└── tests/
    └── ...
```

## 核心设计原则

1. **数据驱动**：Tile定义、物品属性都用数据文件配置，不硬编码
2. **空间局部性**：Chunk保证同一区块的Tile在内存中连续存储
3. **懒加载**：世界按需生成，Chunk按需加载
4. **脏标记传播**：数据变更 → Chunk标记脏 → 重建渲染数据
5. **面向数据设计(DOD)**：ECS架构，组件紧凑存储，系统批量处理
6. **单一职责**：每个System只做一件事，通过EventBus通信

## 开发路线图

| 阶段 | 内容 | 预估 |
|------|------|------|
| **Phase 1** | 世界系统：Tile、Chunk、World、地形生成 | 当前 |
| **Phase 2** | ECS框架 + 物理系统 + 基础渲染 | — |
| **Phase 3** | 玩家：移动、跳跃、挖掘、放置 | — |
| **Phase 4** | 物品系统：背包、掉落、合成 | — |
| **Phase 5** | 敌人AI + 战斗系统 | — |
| **Phase 6** | 液体模拟 + 光照系统 | — |
| **Phase 7** | UI系统 + 存档系统 | — |
| **Phase 8** | 多人联机 | — |
