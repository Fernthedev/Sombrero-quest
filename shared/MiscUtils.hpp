#pragma once

#include <string>
#include <concepts>
#include <cmath>
#include <algorithm>

#ifdef SOMBRERO_DEBUG

#ifdef HAS_CODEGEN
#warning CODEGEN DETECTED
#else
#warning NO CODEGEN DETECTED
#include "extern/beatsaber-hook/shared/utils/il2cpp-type-check.hpp"
#endif

#endif

namespace Sombrero {
    inline static float Clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    inline static float Lerp(float a, float b, float t)
    {
        return a + (b - a) * Clamp01(t);
    }
    
    inline static float PingPong(float t, float length)
    {
        float graph = std::fmod(t, length * 2);
        if (graph > length) graph = ((2 * length) - graph);
        // should yield
        //length_______________
        //                 /\      /\      /
        //                /  \    /  \    /
        //               /    \  /    \  /
        //zero_________ /      \/      \/
        return graph;
    }
}
