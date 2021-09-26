#pragma once

#include "ColorUtils.hpp"

namespace Sombrero {
    struct HSBColor {
        float h, s, b, a;
        constexpr HSBColor(float h, float s, float b, float a) : h(h), s(s), b(b), a(a) {}
        constexpr HSBColor(float h = {}, float s = {}, float b = {}) : h(h), s(s), b(b), a(1.0f) {}

        constexpr HSBColor(const FastColor& color)
        {
            auto temp = FromColor(color);
            h = temp.h;
            s = temp.s;
            b = temp.b;
            a = temp.a;
        }

        inline static std::string HSBColorStr(HSBColor const& color) {
            return "H: " + std::to_string(color.h) + ", S: " + std::to_string(color.s) + ", B:" + std::to_string(color.b) + ", A:" + std::to_string(color.a);
        }

        FastColor ToColor() const
        {
            return ToColor(*this);
        }

        constexpr static HSBColor FromColor(const FastColor& color)
        {
            HSBColor ret = HSBColor(0.0f, 0.0f, 0.0f, color.a);

            float r = color.r;
            float g = color.g;
            float b = color.b;

            float max = std::max(r, std::max(g, b));

            if (max <= 0)
            {
                return ret;
            }

            float min = std::min(r, std::min(g, b));
            float diff = max - min;

            if (max > min)
            {
                if (g == max)
                {
                    ret.h = (b - r) / diff * 60.0f + 120.0f;
                }
                else if (b == max)
                {
                    ret.h = (r - g) / diff * 60.0f + 240.0f;
                }
                else if (b > g)
                {
                    ret.h = (g - b) / diff * 60.0f + 360.0f;
                }
                else
                {
                    ret.h = (g - b) / diff * 60.0f;
                }

                if (ret.h < 0)
                {
                    ret.h = ret.h + 360.0f;
                }
            }
            else
            {
                ret.h = 0;
            }

            ret.h *= 1.0f / 360.0f;
            ret.s = diff / max;
            ret.b = max;

            return ret;
        }

        constexpr static FastColor ToColor(const HSBColor& hsbColor)
        {
            float r = hsbColor.b;
            float g = hsbColor.b;
            float b = hsbColor.b;
            if (hsbColor.s != 0)
            {
                float max = hsbColor.b;
                float diff = hsbColor.b * hsbColor.s;
                float min = hsbColor.b - diff;

                float h = hsbColor.h * 360.0f;

                if (h < 60.0f)
                {
                    r = max;
                    g = h * diff / 60.0f + min;
                    b = min;
                }
                else if (h < 120.0f)
                {
                    r = -(h - 120.0f) * diff / 60.0f + min;
                    g = max;
                    b = min;
                }
                else if (h < 180.0f)
                {
                    r = min;
                    g = max;
                    b = (h - 120.0f) * diff / 60.0f + min;
                }
                else if (h < 240.0f)
                {
                    r = min;
                    g = -(h - 240.0f) * diff / 60.0f + min;
                    b = max;
                }
                else if (h < 300.0f)
                {
                    r = (h - 240.0f) * diff / 60.0f + min;
                    g = min;
                    b = max;
                }
                else if (h <= 360.0f)
                {
                    r = max;
                    g = min;
                    b = -(h - 360.0f) * diff / 60 + min;
                }
                else
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
            }

            return FastColor(Sombrero::Clamp01(r), Sombrero::Clamp01(g), Sombrero::Clamp01(b), hsbColor.a);
        }

        inline std::string toString() const {
            return HSBColorStr(*this);
        }
    };
}
