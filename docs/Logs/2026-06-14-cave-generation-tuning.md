# 2026-06-14 洞穴生成调参日志

## 背景

本次根据需求调整洞穴生成：让洞穴默认生成得更多，并增加几个集中可调的参数，用来控制洞穴数量、大小和边缘破碎程度。

## 改动范围

- `src/World/WorldGenerator.hpp`
  - 新增 `CaveGenerationSettings`。
  - 新增默认参数 `kCaveSettings`。

- `src/World/WorldGenerator.cpp`
  - 洞穴主噪音频率改为受 `size` 控制。
  - 洞穴阈值改为受 `density` 和 `size` 共同影响。
  - 洞穴边界噪音改为受 `edgeRoughness` 控制。
  - 浅层保护深度改为使用 `surfaceProtectionDepth`。

## 新增可调参数

- `density = 1.45`
  - 控制洞穴数量。
  - 大于 `1.0` 会降低挖空阈值，洞穴更多。

- `size = 1.35`
  - 控制洞穴主体大小。
  - 大于 `1.0` 会降低主洞穴噪音频率，生成更大的洞穴区域。

- `edgeRoughness = 1.10`
  - 控制洞穴边缘破碎感。
  - 值越高，边界越不规则。

- `surfaceProtectionDepth = 12`
  - 控制地表下多少格以内不允许洞穴挖空。
  - 当前从之前的浅层保护逻辑进一步放宽，让洞穴能更早出现，但仍避免直接破坏草皮。

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

第一次 Release 链接时 `build/Release/TRlike.exe` 正在运行，导致输出文件被占用。结束该进程后重新编译通过，生成 `build/Release/TRlike.exe`。

