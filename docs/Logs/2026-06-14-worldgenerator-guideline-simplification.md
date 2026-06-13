# 2026-06-14 WorldGenerator guideline 收敛日志

## 背景

本次根据需求清理 `WorldGenerator` 中不属于 guideline 的额外洞穴设计，只保留最少代码来实现当前文档目标。

## 删除的多余设计

- 洞穴锚点覆盖系统。
- coverage 硬/软门槛。
- 分离强度系统。
- 区域均值校正系统。
- 多组洞穴密度、大小、均匀化参数。
- 额外的区域缓存和分布单元插值逻辑。

## 保留的 guideline 内容

- A 噪音：大间隙、大尺度、多 octave，控制整体洞穴。
- B 噪音：小间隙、小尺度、多 octave，控制不规则边界。
- `A - B` 大于阈值时挖为空气。
- 每个 chunk 的阈值由当前 chunk 及附近 chunk 的 `A - B` 采样平均值决定。
- 附近 chunk 使用较小权重参与阈值计算。
- chunk 间阈值使用平滑双线性插值过渡。
- 泥土层允许生成洞穴，但浅层会增加阈值惩罚，让洞穴明显更少。

## 当前可调参数

- `thresholdStdDevScale = 1.15`
  - 阈值为局部平均值加标准差系数，用来让洞穴比例靠近 guideline 中的约 10% 目标。

- `neighborRadius = 1`
  - 阈值计算采样当前 chunk 周围 1 格范围。

- `samplesPerAxis = 5`
  - 每个 chunk 单轴采样 5 个点。

- `shallowPenaltyDepth = 32`
  - 地表下 32 格内视为浅层惩罚区。

- `shallowPenalty = 0.16`
  - 浅层洞穴额外提高的阈值强度。

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

