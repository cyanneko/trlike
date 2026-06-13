# 2026-06-14 洞穴 Coverage 软门槛日志

## 背景

上一次分析发现洞穴密度偏低、仍有局部集中，主要原因是低 coverage 区域被硬 `continue` 跳过，导致自适应阈值无法补洞；同时锚点在分区内随机范围过大，会让相邻锚点偶尔挤在一起。

## 改动范围

- `src/World/WorldGenerator.hpp`
  - `minimumAnchorCoverage` 从 `0.16` 下调为 `0.10`。
  - `anchorInnerRadius` 从 `0.10` 调整为 `0.12`。
  - `anchorOuterRadius` 从 `0.54` 调整为 `0.62`。
  - `separationStrength` 从 `0.65` 下调为 `0.48`。

- `src/World/WorldGenerator.cpp`
  - 移除 `coverage < minimumAnchorCoverage` 时直接跳过挖洞的硬门槛。
  - 低 coverage 区域现在使用 `lowCoverage` 增加阈值，作为软惩罚。
  - 分区锚点随机范围从 `20%~80%` 收窄到 `35%~65%`，让锚点更接近分区中心。

## 当前效果

- 低 coverage 区域不再完全禁止洞穴生成，仍然有机会被自适应阈值和噪音补出小洞。
- 锚点更均匀地落在各自分区中心附近，减少锚点挤在一起造成的集中。
- 分离强度降低后，洞穴总量会比上一版稍微恢复，但低覆盖位置仍然有额外阈值惩罚，不会直接回到过度连通状态。

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

