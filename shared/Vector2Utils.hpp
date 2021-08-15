#pragma once

#include "MiscUtils.hpp"

#include "beatsaber-hook/shared/utils/typedefs.h"
#ifdef HAS_CODEGEN
#include "UnityEngine/Vector2.hpp"
#endif

#define CONSTEXPR_GETTER(name, ...) \
constexpr static inline FastVector2 name() {\
    return __VA_ARGS__;\
}

#ifndef HAS_CODEGEN
// TODO: Will this break things?
namespace UnityEngine
{
    struct Vector2
    {
        float x, y;
    }
}
#endif

namespace Sombrero {

    struct FastVector2;

    inline static std::string vector2Str(UnityEngine::Vector2 const &vector2)
    {
        return std::to_string(vector2.x) + ", " + std::to_string(vector2.y);
    }

    struct FastVector2 : public UnityEngine::Vector2 {
    public:
        // Implicit convert of vector
        constexpr FastVector2(const Vector2& vector) : Vector2(vector.x, vector.y) {}

        constexpr FastVector2(float x = 0.0f, float y = 0.0f) : Vector2(x, y) {}

        CONSTEXPR_GETTER(one, {1.0f, 1.0f})
        CONSTEXPR_GETTER(zero, {0.0f, 0.0f})

        CONSTEXPR_GETTER(down, {0.0f, -1.0f})
        CONSTEXPR_GETTER(up, {0.0f, 1.0f})
        CONSTEXPR_GETTER(left, {-1.0f, 0.0f})
        CONSTEXPR_GETTER(right, {1.0f, 0.0f})

        inline std::string toString() {
            return vector2Str(*this);
        }

        inline static FastVector2 Lerp(FastVector2 const& a, FastVector2 const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static FastVector2 LerpUnclamped(FastVector2 const& a, FastVector2 const& b, float const& t)
        {
            return FastVector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        float Magnitude() {
            return std::sqrt((x * x) + (y * y));
        }

        float Distance(const FastVector2& b) {
            float dx = x - b.x;
            float dy = y - b.y;
            return sqrt(dx * dx + dy * dy);
        }

        static inline FastVector2 Normalize(FastVector2& vec) {
            float magnitude = vec.Magnitude();
            if (magnitude == 0.0f) return {0.0f, 0.0f};
            return vec / magnitude;
        }

        inline void Normalize() {
            NormalizeFast();
        }

        // In case codegen method takes over
        inline void NormalizeFast() {
            float magnitude = Magnitude();
            if (magnitude == 0.0f) {
                x = 0.0f;
                y = 0.0f;
            }
            *this /= magnitude;
        }

#define operatorOverload(name, operatore) \
        FastVector2 operator operatore(const FastVector2& b) const { \
            return FastVector2(this->x operatore b.x, this->y operatore this->y); \
        }                                \
        FastVector2 operator operatore(float const& b) const { \
            return FastVector2(this->x operatore b, this->y operatore b); \
        }                                 \
        FastVector2& operator operatore##=(float const& bb) {  \
            x operatore##= bb;                       \
            y operatore##= bb;                        \
            return *this; \
        }                                 \
        FastVector2& operator operatore##=(const FastVector2& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            return *this; \
        }


        operatorOverload(add, +)
        operatorOverload(subtract, -)
        operatorOverload(multiply, *)
        operatorOverload(divide, /)

#undef operatorOverload

        FastVector2 operator-() const {
            return FastVector2(-x, -y);
        }


        bool operator ==(const FastVector2& lhs) {
            return lhs.x == x && lhs.y == y;
        }

        inline bool operator !=(const FastVector2& lhs) {
            return lhs.x != x || lhs.y != y;
        }

        float& operator[](int i) {
            return (&x)[i];
        }
    };

#define operatorOverload(name, operator) \
    static FastVector2 vector2##name(UnityEngine::Vector2 const &a, UnityEngine::Vector2 const &b) \
    { \
        return FastVector2(a.x operator b.x, a.y operator b.y); \
    } \
    static FastVector2 vector2##name (UnityEngine::Vector2 const& a, float const& b) { \
        return FastVector2(a.x operator b, a.y operator b); \
    } \

    operatorOverload(add, +)
    operatorOverload(subtract, -)
    operatorOverload(multiply, *)
    operatorOverload(scale, *)
    operatorOverload(divide, /)

#undef operatorOverload

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Vector2) == sizeof(FastVector2));
#endif
}
DEFINE_IL2CPP_ARG_TYPE(Sombrero::FastVector2, "UnityEngine", "Vector2");
#undef CONSTEXPR_GETTER