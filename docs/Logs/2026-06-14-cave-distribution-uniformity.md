# 2026-06-14 洞穴分布均匀化日志

## 背景

本次根据需求调整洞穴生成，使洞穴在地图中分布得更加均匀，减少纯噪音生成时可能出现的大面积空洞缺失或过度聚集。

## 改动范围

- `src/World/WorldGenerator.hpp`
  - 在 `CaveGenerationSettings` 中新增 `uniformity`。
  - 在 `CaveGenerationSettings` 中新增 `distributionCellSize`。

- `src/World/WorldGenerator.cpp`
  - 新增 `SmoothStep` 辅助函数。
  - 在 `GenerateCaves` 中加入分区覆盖逻辑。
  - 每个固定大小的逻辑区域都会生成一个确定性洞穴锚点。
  - 洞穴判定会根据当前位置距离附近锚点的覆盖程度调整阈值。

## 新增可调参数

- `uniformity = 0.75`
  - 控制洞穴分布均匀化强度。
  - `0.0` 接近纯噪音分布。
  - `1.0` 会更强地把洞穴拉向均匀分布。

- `distributionCellSize = 150`
  - 控制洞穴分布区域的逻辑尺寸。
  - 值越小，洞穴锚点越密，分布更密集。
  - 值越大，洞穴锚点越疏，分布更开阔。

## 当前生成逻辑

洞穴仍然以 `A - B > threshold` 为基础：

- A 噪音控制洞穴主体。
- B 噪音控制边缘破碎。
- 分区锚点只调整局部阈值和洞穴值，负责大尺度均匀分布。

这样不会把洞穴变成规整网格，而是在保持噪音形状的同时，让各区域都有稳定的生成机会。

## 验证

已编译 Debug：

```powershell
cmake --build build --config Debug
```

结果：通过，生成 `build/Debug/TRlike.exe`。

已编译 Release：

```powershell
cmake --build build --config Release
```

结果：通过，生成 `build/Release/TRlike.exe`。

