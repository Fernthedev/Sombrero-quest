#pragma once

#include "MiscUtils.hpp"

#include "UnityEngine/Vector2.hpp"

namespace Sombrero {
    // Declaration of the concept "Hashable", which is satisfied by any type 'T'
// such that for values 'a' of type 'T', the expression std::hash<T>{}(a)
// compiles and its result is convertible to std::size_t
    template<typename T>
    concept Vector2Derive = requires() {
        std::is_base_of<UnityEngine::Vector2, T>::value;
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

    struct FastVector2 : public UnityEngine::Vector2 {
    public:
        // Implicit convert of vector
        FastVector2(const Vector2& vector) : Vector2(vector.x, vector.y) {} // x(vector.x), y(vector.y), z(vector.z) {}

        explicit FastVector2(float x = 0.0f, float y = 0.0f) : Vector2(x, y) {}

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

#define operatorOverload(name, operatore) \
        FastVector2 operator operatore(const FastVector2& b) const { \
            return vector2##name(*this, b); \
        }                                \
        FastVector2 operator operatore(float const& b) const { \
            return vector2##name(*this, b); \
        }                                \

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
            return lhs.x != x && lhs.y != y;
        }
    };

    static_assert(sizeof(UnityEngine::Vector2) == sizeof(FastVector2));
}