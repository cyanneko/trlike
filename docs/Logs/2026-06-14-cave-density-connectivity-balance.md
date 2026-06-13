# 2026-06-14 洞穴数量与连通性平衡日志

## 背景

上一次洞穴均匀化后，洞穴数量偏多，并且不同区域之间过于容易连通；同时仍有部分区域缺少洞穴分布。本次调整目标是：

- 降低洞穴总量。
- 减少洞穴之间的长距离连通。
- 保持更稳定的区域覆盖，避免大片区域完全没有洞穴。

## 改动范围

- `src/World/WorldGenerator.hpp`
  - 降低 `density` 默认值。
  - 降低 `size` 默认值，减少大块连续洞穴。
  - 提高 `uniformity`，强化区域覆盖。
  - 缩小 `distributionCellSize`，让洞穴锚点更密，减少无洞区域。
  - 新增 `minimumAnchorCoverage`。
  - 新增 `anchorInnerRadius` 和 `anchorOuterRadius`。
  - 新增 `separationStrength`。

- `src/World/WorldGenerator.cpp`
  - 低于 `minimumAnchorCoverage` 的区域直接保持实体，不再参与洞穴挖空。
  - 洞穴覆盖半径改为由 `anchorInnerRadius` / `anchorOuterRadius` 控制。
  - 将覆盖值归一化为 `gatedCoverage`，只在锚点有效范围内降低阈值。
  - 提高基础挖空阈值，降低整体洞穴数量。
  - 在覆盖较低的位置额外提高阈值，用 `separationStrength` 保留实体隔断。

## 当前关键参数

- `density = 1.08`
  - 从之前偏激进的数量配置下调，减少洞穴总量。

- `size = 0.95`
  - 降低主洞穴尺度，减少大范围连成片的洞穴。

- `uniformity = 0.90`
  - 提高区域覆盖强度，让每个分区更稳定地有洞穴候选。

- `distributionCellSize = 120`
  - 分区尺寸变小，锚点更密，减少无洞区域。

- `minimumAnchorCoverage = 0.16`
  - 覆盖不足的位置不挖空，用来打断洞穴连接。

- `anchorInnerRadius = 0.10`
  - 锚点核心范围。

- `anchorOuterRadius = 0.54`
  - 锚点最大影响范围。

- `separationStrength = 0.65`
  - 增强锚点之间的实体隔断。

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

