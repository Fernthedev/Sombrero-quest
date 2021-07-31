#pragma once

namespace Sombrero {
    static float Clamp01(float value)
    {
        bool flag = value < 0.0f;
        float result;
        if (flag)
        {
            result = 0.0f;
        }
        else
        {
            bool flag2 = value > 1.0f;
            if (flag2)
            {
                result = 1.0f;
            }
            else
            {
                result = value;
            }
        }
        return result;
    }

    inline static float Lerp(float a, float b, float t)
    {
        return a + (b - a) * Clamp01(t);
    }
}
