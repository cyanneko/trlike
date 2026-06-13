# 2026-06-14 洞穴 Chunk 自适应阈值日志

## 背景

本次根据需求调整洞穴阈值：不再只使用固定阈值，而是让每个 chunk 的洞穴阈值与该 chunk 及附近 chunk 的 `A - B` 平均值相关，并在 chunk 之间平滑过渡。

## 改动范围

- `src/World/WorldGenerator.hpp`
  - 在 `CaveGenerationSettings` 中新增自适应阈值参数。

- `src/World/WorldGenerator.cpp`
  - 抽出 `rawCaveSignal`，统一计算洞穴原始信号 `A - B`。
  - 新增附近 chunk 的 `A - B` 均值采样。
  - 为当前 chunk 周围 3x3 chunk 预计算阈值偏移。
  - 在 tile 级别对相邻 chunk 的阈值偏移做平滑双线性插值。
  - 将插值得到的自适应偏移叠加到原有洞穴阈值上。

## 新增参数

- `adaptiveThresholdTarget = 0.20`
  - 目标 `A - B` 平均值。

- `adaptiveThresholdStrength = 0.55`
  - 控制 chunk 平均值对阈值的影响强度。

- `adaptiveThresholdMaxOffset = 0.08`
  - 限制自适应阈值偏移的最大幅度，避免局部过度修正。

- `adaptiveThresholdNeighborhood = 1`
  - 采样当前 chunk 周围 1 格范围，即 3x3 chunk。

- `adaptiveThresholdSamplesPerAxis = 4`
  - 每个 chunk 单轴采样 4 个点，即每个 chunk 采样 16 点。

## 当前效果

- 如果某个区域的 `A - B` 平均值偏高，说明它天然更容易生成洞穴，阈值会被抬高，减少过度连通。
- 如果某个区域的 `A - B` 平均值偏低，说明它天然缺少洞穴，阈值会被降低，补足洞穴分布。
- chunk 之间不是硬切换，而是按世界位置平滑插值，避免边界处出现突变。

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

