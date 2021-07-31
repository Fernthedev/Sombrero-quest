#pragma once

#include "MiscUtils.hpp"

#ifdef HAS_CODEGEN
#include "UnityEngine/Vector2.hpp"
#endif

namespace Sombrero {

    struct FastVector2;

    template<typename T>
    concept Vector2Derive = requires() {
#ifdef HAS_CODEGEN
        std::is_base_of<UnityEngine::Vector2, T>::value;
#else
        std::is_base_of<FastVector2, T>::value;
#endif
        !std::is_pointer<T>::value;
    };

#define operatorOverload(name, operator) \
    template<Vector2Derive T> \
    static T vector2##name (T const& a, T const& b) { \
        return T(a.x operator b.x, a.y operator b.y); \
    } \
    template<Vector2Derive T> \
    static T vector2##name (T const& a, float const& b) { \
        return T(a.x operator b, a.y operator b); \
    } \


    template<Vector2Derive T>
    inline static std::string vector2Str(T const& vector2) {
        return std::to_string(vector2.x) + ", " + std::to_string(vector2.y);
    }

    operatorOverload(add, +)
    operatorOverload(subtract, -)
    operatorOverload(multiply, *)
    operatorOverload(scale, *)
    operatorOverload(divide, /)

#undef operatorOverload

#ifdef HAS_CODEGEN
    struct FastVector2 : public UnityEngine::Vector2 {
#else
      struct FastVector2 {
          float x,y;
#endif
    public:

#ifdef HAS_CODEGEN
        // Implicit convert of vector
        FastVector2(const Vector2& vector) : Vector2(vector.x, vector.y) {}

        FastVector2(float x = 0.0f, float y = 0.0f) : Vector2(x, y) {}
#else
        FastVector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
#endif



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
            return vector2##name(*this, b); \
        }                                \
        FastVector2 operator operatore(float const& b) const { \
            return vector2##name(*this, b); \
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


        template<Vector2Derive T>
        bool operator ==(const T& lhs) {
            return lhs.x == x && lhs.y == y;
        }

        template<Vector2Derive T>
        inline bool operator !=(const T& lhs) {
            return !(this == lhs);
        }
    };

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Vector2) == sizeof(FastVector2));
#else
    DEFINE_IL2CPP_ARG_TYPE(FastVector2, "UnityEngine", "Vector2");
#endif
}