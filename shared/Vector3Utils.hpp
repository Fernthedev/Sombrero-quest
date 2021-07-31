#pragma once

#include "MiscUtils.hpp"

#include "UnityEngine/Vector3.hpp"

namespace Sombrero {
    // Declaration of the concept "Hashable", which is satisfied by any type 'T'
// such that for values 'a' of type 'T', the expression std::hash<T>{}(a)
// compiles and its result is convertible to std::size_t
    template<typename T>
    concept Vector3Derive = requires() {
        std::is_base_of<UnityEngine::Vector3, T>::value;
        !std::is_pointer<T>::value;
    };

#define operatorOverload(name, operator) \
    template<Vector3Derive T> \
    static T vector3##name (T const& a, T const& b) { \
        return T(a.x operator b.x, a.y operator b.y, a.z operator b.z); \
    } \
    template<Vector3Derive T> \
    static T vector3##name (T const& a, float const& b) { \
        return T(a.x operator b, a.y operator b, a.z operator b); \
    } \



    template<Vector3Derive T>
    inline std::string vector3Str(T const& vector3) {
        return std::to_string(vector3.x) + ", " + std::to_string(vector3.y) + ", " + std::to_string(vector3.z);
    }

    operatorOverload(add, +)
    operatorOverload(subtract, -)
    operatorOverload(multiply, *)
    operatorOverload(scale, *)
    operatorOverload(divide, /)

#undef operatorOverload

    struct FastVector3 : public UnityEngine::Vector3 {
    public:
        // Implicit convert of vector
        FastVector3(const Vector3& vector) : Vector3(vector.x, vector.y, vector.z) {} // x(vector.x), y(vector.y), z(vector.z) {}

        explicit FastVector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : Vector3(x, y, z) {}

        inline std::string toString() {
            return vector3Str(*this);
        }

        inline static FastVector3 Lerp(FastVector3 const& a, FastVector3 const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static FastVector3 LerpUnclamped(FastVector3 const& a, FastVector3 const& b, float const& t)
        {
            return FastVector3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
        }

#define operatorOverload(name, operatore) \
        FastVector3 operator operatore(const FastVector3& b) const { \
            return vector3##name(*this, b); \
        }                                \
        FastVector3 operator operatore(float const& b) const { \
            return vector3##name(*this, b); \
        }                                \

        operatorOverload(add, +)
        operatorOverload(subtract, -)
        operatorOverload(multiply, *)
        operatorOverload(divide, /)

#undef operatorOverload

        FastVector3 operator-() const {
            return FastVector3(-x, -y, -z);
        }


        template<Vector3Derive T>
        bool operator ==(const T& lhs) {
            return lhs.x == x && lhs.y == y && lhs.z == z;
        }

        template<Vector3Derive T>
        inline bool operator !=(const T& lhs) {
            return lhs.x != x && lhs.y != y && lhs.z != z;
        }
    };

    static_assert(sizeof(UnityEngine::Vector3) == sizeof(FastVector3));
}