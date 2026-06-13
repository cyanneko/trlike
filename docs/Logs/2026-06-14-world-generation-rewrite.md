# 2026-06-14 世界生成重写日志

## 背景

根据 `docs/Human/关于地形生成——guideline` 和整理后的 `docs/VibeDocing/关于地形生成——理解.md`，本次将世界生成逻辑重写为第一版有限地图生成管线。

## 改动范围

- `src/Core/Constants.hpp`
  - 将世界逻辑范围设为 `x: -2000~2000, y: -700~300`。
  - 存储尺寸对应为 `4001 x 1001`。
  - 将逻辑 `y = 0` 映射到数组坐标 `WORLD_SURFACE_Y = 700`。
  - Chunk 数量改为向上取整，避免边界不是 32 整倍数时丢失最后一行/列。

- `src/World/WorldGenerator.hpp`
  - 移除旧的矿物、背景墙、复杂洞穴阶段声明。
  - 增加一维/二维 Perlin 噪音参数结构。
  - 保留 `GenerateWorld` 和 `GenerateChunk` 作为外部入口。

- `src/World/WorldGenerator.cpp`
  - 实现确定性一维 Perlin 噪音。
  - 实现确定性二维 Perlin 噪音。
  - 实现一维/二维分形 Perlin 叠加。
  - 二维噪音支持旋转。
  - 地表高度由多个正弦波叠加，再叠加一维 Perlin 噪音生成。
  - 地形先填充为空气/石头，再用二维噪音把靠近地表的部分石头转为泥土。
  - 地表最上层泥土转为草方块。
  - 洞穴改为两个二维噪音场 `A - B > threshold` 的简单生成方式。

## 当前保留和暂缓

当前版本只实现 guideline 中明确要求的第一阶段：

- 有限地图边界。
- 地平线生成。
- 石头、泥土、草方块生成。
- 简单洞穴生成。

暂缓内容：

- 矿物。
- 生物群系。
- 树木、植物和装饰物。
- 建筑/遗迹。
- 水、岩浆等流体。
- 旧版蛇形洞穴、洞穴入口和大洞室系统。

## 验证

已重新编译：

```powershell
cmake --build build --config Debug
```

结果：编译通过，生成 `build/Debug/TRlike.exe`。

## 备注

根目录 `README` 已要求所有 agent 在重大代码改动后写入 `docs/Logs`。后续进行版本迭代后，应默认执行“编译验证 + 写日志”这一步。

