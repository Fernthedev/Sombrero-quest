#pragma once

#include <limits>
#include <string>
#include <concepts>
#include <cmath>
#include <algorithm>
#include <functional>
#include <type_traits>
#include "Concepts.hpp"

#ifdef SOMBRERO_DEBUG

#ifdef HAS_CODEGEN
#warning CODEGEN DETECTED
#else
#warning NO CODEGEN DETECTED
#include "beatsaber-hook/shared/types.hpp"
#endif

#endif

namespace Sombrero {

    // Opt-in policies for unordered keys. Unity-style approximate operator==
    // is not an equivalence relation and cannot support a default std::hash.
    template<class T> struct ExactHash;
    template<class T> struct ExactEqual;
    
    constexpr float Clamp01(float value)
    {
        return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    }

    template<Sombrero::Lerpable T>
    constexpr T Lerp(T a, T b, float t)
    {
        return T::Lerp(a, b, Clamp01(t));
    }

    template<Sombrero::Slerpable T>
    constexpr T Slerp(T a, T b, float t)
    {
        return T::Slerp(a, b, Clamp01(t));
    }
    
    inline static float PingPong(float t, float length)
    {
        const float period = length * 2.0f;
        float repeated = t - std::floor(t / period) * period;
        repeated = repeated < 0.0f ? 0.0f : (repeated > period ? period : repeated);
        return length - std::abs(repeated - length);
    }

    // Credit to sc2ad for making the sqrt and pow constexpr
    namespace detail {
        constexpr float sqrt(float x) {
            // Constant evaluation only; runtime uses the platform square root.
            double curr = x >= 1.0f ? x : 1.0;
            for (;;) {
                const double next = 0.5 * (curr + double(x) / curr);
                if (next >= curr) return static_cast<float>(curr);
                curr = next;
            }
        }

        // Hash keys need an equivalence relation even for NaN components.
        constexpr bool FloatKeyEqual(float a, float b) {
            return a == b || (a != a && b != b);
        }

        inline size_t FloatKeyHash(float value) noexcept {
            return std::hash<float>{}(value == value ? value : std::numeric_limits<float>::quiet_NaN());
        }
    }

    constexpr float power(float x, int p) 
    {
        // Unsigned subtraction also handles INT_MIN without signed overflow.
        unsigned int exponent = p < 0 ? 0u - static_cast<unsigned int>(p) : static_cast<unsigned int>(p);
        // Invert first so a representable negative power does not become zero
        // merely because the corresponding positive power overflows.
        double factor = p < 0 ? 1.0 / static_cast<double>(x) : static_cast<double>(x);
        double result = 1.0;
        while (exponent != 0) {
            if ((exponent & 1u) != 0) result *= factor;
            exponent >>= 1;
            if (exponent != 0) factor *= factor;
        }
        return static_cast<float>(result);
    }

    constexpr float sqroot(float x) 
    {
        if (!std::is_constant_evaluated())
            return static_cast<float>(std::sqrt(static_cast<double>(x)));
        if (x == 0.0f || x == std::numeric_limits<float>::infinity()) return x;
        return x > 0.0f ? detail::sqrt(x) : std::numeric_limits<float>::quiet_NaN();
    }

    // Convert degrees to radians
    constexpr double degreesToRadians(double degrees) {
      return degrees * M_PI / 180.0;
    }
}
