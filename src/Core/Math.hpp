#pragma once

#include <cmath>
#include <algorithm>
#include <concepts>
#include <random>

namespace TR::Math {

// ============================================================
// 通用数学工具
// ============================================================

template<std::floating_point T>
constexpr T Pi = T(3.14159265358979323846);

/// 将值限制在 [min, max] 范围内
template<typename T>
constexpr T Clamp(const T& value, const T& min, const T& max) {
    return std::min(std::max(value, min), max);
}

/// 线性插值
template<std::floating_point T>
constexpr T Lerp(T a, T b, T t) {
    return a + (b - a) * t;
}

/// 将值从一个范围映射到另一个范围
template<std::floating_point T>
constexpr T Remap(T value, T inMin, T inMax, T outMin, T outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

/// 取模（正数结果）
constexpr int Mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

/// 向下取整到2的幂
constexpr uint32_t NextPowerOfTwo(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

/// 简单的伪随机数生成器 (用于世界生成，确定性)
class XorShift64 {
public:
    explicit XorShift64(uint64_t seed = 123456789) : m_state(seed) {}

    uint64_t Next() {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 7;
        m_state ^= m_state << 17;
        return m_state;
    }

    /// 返回 [0, 1) 的浮点数
    double NextDouble() {
        return static_cast<double>(Next()) / static_cast<double>(UINT64_MAX);
    }

    /// 返回 [min, max] 的整数
    int NextInt(int min, int max) {
        return min + static_cast<int>(Next() % (max - min + 1));
    }

    void Seed(uint64_t seed) { m_state = seed; }

private:
    uint64_t m_state;
};

} // namespace TR::Math
