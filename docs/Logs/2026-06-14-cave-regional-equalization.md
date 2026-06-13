# 2026-06-14 洞穴区域均值校正日志

## 背景

继续调整洞穴生成的均匀性。此前已经做了锚点居中、coverage 软门槛和 chunk 自适应阈值，但洞穴仍可能因为局部噪音和锚点覆盖差异出现偏集中。此次将均匀化进一步推进到分布单元级别。

## 改动范围

- `src/World/WorldGenerator.hpp`
  - 新增区域均值校正参数：
    - `regionalEqualizationTarget`
    - `regionalEqualizationStrength`
    - `regionalEqualizationMaxOffset`
    - `regionalEqualizationSamplesPerAxis`

- `src/World/WorldGenerator.cpp`
  - 引入 `<vector>`，用于缓存当前 chunk 附近分布单元的阈值偏移。
  - `distributionCoverage` 改为支持 double 坐标，方便区域采样。
  - 新增 `caveScore`，统一计算最终用于洞穴判断的局部分数。
  - 新增分布单元内采样逻辑，计算每个单元的平均 cave score。
  - 根据单元平均分数和目标值计算阈值偏移：
    - 分数偏高的单元抬高阈值。
    - 分数偏低的单元降低阈值。
  - 对相邻分布单元的阈值偏移做平滑双线性插值。

## 新增参数

- `regionalEqualizationTarget = 0.22`
  - 区域洞穴分数的目标均值。

- `regionalEqualizationStrength = 0.70`
  - 区域均值对阈值偏移的影响强度。

- `regionalEqualizationMaxOffset = 0.075`
  - 区域阈值偏移的最大幅度，避免单元过度修正。

- `regionalEqualizationSamplesPerAxis = 4`
  - 每个分布单元单轴采样 4 个点，即每单元 16 个采样点。

## 当前效果

- 每个分布单元都会根据自身洞穴分数被局部修正。
- 高洞穴倾向区域会被压低生成量。
- 低洞穴倾向区域会被补强生成机会。
- 修正值在单元之间平滑过渡，避免硬边界。

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

