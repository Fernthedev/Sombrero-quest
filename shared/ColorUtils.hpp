#pragma once

#include "MiscUtils.hpp"

#ifdef HAS_CODEGEN
#include "UnityEngine/Color.hpp"
#endif

namespace Sombrero {

    struct FastColor;

    template<typename T>
    concept ColorDerive = requires() {
#ifdef HAS_CODEGEN
        std::is_base_of<UnityEngine::Color, T>::value;
#else
        std::is_base_of<FastColor, T>::value;
#endif
        !std::is_pointer<T>::value;
    };

#define operatorOverload(name, operator) \
    template<ColorDerive T> \
    static T Color##name (T const& a, T const& b) { \
        return T(a.r operator b.r, a.g operator b.g, a.b operator b.b); \
    } \
    template<ColorDerive T> \
    static T Color##name (T const& a, float const& b) { \
        return T(a.r operator b, a.g operator b, a.b operator b); \
    } \


    template<ColorDerive T>
    inline static std::string ColorStr(T const& color) {
        return "r: " + std::to_string(color.r) + ", g: " + std::to_string(color.g) + ", b:" + std::to_string(color.b);
    }

    operatorOverload(add, +)
    operatorOverload(subtract, -)
    operatorOverload(multiply, *)
    operatorOverload(scale, *)
    operatorOverload(divide, /)

#undef operatorOverload

    inline static float GammaToLinearSpace(float gamma) {
        return 255.0f * std::pow(gamma / 255.0f, 2.2f);
    }

    inline static float LinearToGammaSpace(float linear) {
        return 255.0f * std::pow(linear / 255.0f, 1.0f/2.2f);
    }

    template<ColorDerive T>
    inline static T ColorLinear(T const& a) {
        return T(GammaToLinearSpace(a.r), GammaToLinearSpace(a.g), GammaToLinearSpace(a.b), a.a);
    }

    template<ColorDerive T>
    inline static T ColorAlpha(T const& a, float alpha) {
        a.a = alpha;
        return a;
    }


    static void RGBToHSVHelper(float offset, float dominantcolor, float colorone, float colortwo, float& H, float& S, float& V)
    {
        V = dominantcolor;
        bool flag = V != 0.0f;
        if (flag)
        {
            bool flag2 = colorone > colortwo;
            float num;
            if (flag2)
            {
                num = colortwo;
            }
            else
            {
                num = colorone;
            }
            float num2 = V - num;
            bool flag3 = num2 != 0.0f;
            if (flag3)
            {
                S = num2 / V;
                H = offset + (colorone - colortwo) / num2;
            }
            else
            {
                S = 0.0f;
                H = offset + (colorone - colortwo);
            }
            H /= 6.0f;
            bool flag4 = H < 0.0f;
            if (flag4)
            {
                H += 1.0f;
            }
        }
        else
        {
            S = 0.0f;
            H = 0.0f;
        }
    }

    template<ColorDerive T>
    static void ColorRGBToHSV(T const& rgbColor, float& H, float& S, float& V)
    {
        bool flag = rgbColor.b > rgbColor.g && rgbColor.b > rgbColor.r;
        if (flag)
        {
            RGBToHSVHelper(4.0f, rgbColor.b, rgbColor.r, rgbColor.g, H, S, V);
        }
        else
        {
            bool flag2 = rgbColor.g > rgbColor.r;
            if (flag2)
            {
                RGBToHSVHelper(2.0f, rgbColor.g, rgbColor.b, rgbColor.r, H, S, V);
            }
            else
            {
                RGBToHSVHelper(0.0f, rgbColor.r, rgbColor.g, rgbColor.b, H, S, V);
            }
        }
    }

    template<ColorDerive T>
    static T ColorHSVToRGB(float H, float S, float V, bool hdr = true)
    {
        auto white = T(1.0f, 1.0f, 1.0f, 1.0f);

        bool flag = S == 0.0f;
        if (flag)
        {
            white.r = V;
            white.g = V;
            white.b = V;
        }
        else
        {
            bool flag2 = V == 0.0f;
            if (flag2)
            {
                white.r = 0.0f;
                white.g = 0.0f;
                white.b = 0.0f;
            }
            else
            {
                white.r = 0.0f;
                white.g = 0.0f;
                white.b = 0.0f;
                float num = H * 6.0f;
                int num2 = (int)std::floor(num);
                float num3 = num - (float)num2;
                float num4 = V * (1.0f - S);
                float num5 = V * (1.0f - S * num3);
                float num6 = V * (1.0f - S * (1.0f - num3));
                switch (num2)
                {
                    case -1:
                        white.r = V;
                        white.g = num4;
                        white.b = num5;
                        break;
                    case 0:
                        white.r = V;
                        white.g = num6;
                        white.b = num4;
                        break;
                    case 1:
                        white.r = num5;
                        white.g = V;
                        white.b = num4;
                        break;
                    case 2:
                        white.r = num4;
                        white.g = V;
                        white.b = num6;
                        break;
                    case 3:
                        white.r = num4;
                        white.g = num5;
                        white.b = V;
                        break;
                    case 4:
                        white.r = num6;
                        white.g = num4;
                        white.b = V;
                        break;
                    case 5:
                        white.r = V;
                        white.g = num4;
                        white.b = num5;
                        break;
                    case 6:
                        white.r = V;
                        white.g = num6;
                        white.b = num4;
                        break;
                }
                bool flag3 = !hdr;
                if (flag3)
                {
                    white.r = std::clamp(white.r, 0.0f, 1.0f);
                    white.g = std::clamp(white.g, 0.0f, 1.0f);
                    white.b = std::clamp(white.b, 0.0f, 1.0f);
                }
            }
        }
        return white;
    }

#ifdef HAS_CODEGEN
    struct FastColor : public UnityEngine::Color {
#else
    struct FastColor {
    float r, g, b, a;
#endif
    public:

#ifdef HAS_CODEGEN
        // Implicit convert of vector
        FastColor(const Color& vector) : Color(vector.r, vector.g, vector.b) {}

        explicit FastColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f) : Color(r, g, b, a) {}
#else
        explicit FastColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f) : r(r), g(g), b(b), a(a) {}
#endif




        inline std::string toString() {
            return ColorStr(*this);
        }

        inline static FastColor Lerp(FastColor const& a, FastColor const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static FastColor LerpUnclamped(FastColor const& a, FastColor const& b, float const& t)
        {
            return FastColor(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t);
        }

        // static public System.Void RGBToHSV(UnityEngine.Color rgbColor, out System.Single H, out System.Single S, out System.Single V)
        // Offset: 0x17D43E0
        static void RGBToHSV(FastColor const& rgbColor, float& H, float& S, float& V) {
            return ColorRGBToHSV(rgbColor, H, S, V);
        }

        // static public UnityEngine.Color HSVToRGB(System.Single H, System.Single S, System.Single V)
        // Offset: 0x17D4568
        static FastColor HSVToRGB(float H, float S, float V) {
            return ColorHSVToRGB<FastColor>(H, S, V);
        }


        FastColor Linear() {
            return FastColor(GammaToLinearSpace(r), GammaToLinearSpace(g), GammaToLinearSpace(b), a);
        }


        FastColor Alpha(float alpha) {
            return FastColor(r, g, b, alpha);
        }

#define operatorOverload(name, operatore) \
        FastColor operator operatore(const FastColor& b) const { \
            return Color##name(*this, b); \
        }                                \
        FastColor operator operatore(float const& b) const { \
            return Color##name(*this, b); \
        }                                \

        operatorOverload(add, +)
        operatorOverload(subtract, -)
        operatorOverload(multiply, *)
        operatorOverload(divide, /)

#undef operatorOverload

        template<ColorDerive T>
        bool operator ==(const T& lhs) {
            return lhs.r == r && lhs.g == g && lhs.b == b && lhs.a == a;
        }

        template<ColorDerive T>
        inline bool operator !=(const T& lhs) {
            return lhs.r != r && lhs.g != g && lhs.b != b && lhs.a != a;
        }
    };

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Color) == sizeof(FastColor));
#else
DEFINE_IL2CPP_ARG_TYPE(FastColor, "UnityEngine", "Color");
#endif
}