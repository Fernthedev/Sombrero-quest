#pragma once

#include "MiscUtils.hpp"
#include "Vector3Utils.hpp"

#include <utility>

#include "beatsaber-hook/shared/utils/typedefs.h"
#ifdef HAS_CODEGEN
#include "UnityEngine/Quaternion.hpp"
#endif

#ifndef HAS_CODEGEN
// TODO: Will this break things?
namespace UnityEngine
{
    struct Quaternion
    {
        float x, y, z, w;
        
        constexpr Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    };
}
#endif

namespace Sombrero {

    struct FastQuaternion;

    constexpr static UnityEngine::Quaternion QuaternionMultiply(UnityEngine::Quaternion const &lhs, UnityEngine::Quaternion const &rhs)
    {
        return UnityEngine::Quaternion(lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
    }

    constexpr static FastVector3 QuaternionMultiply(UnityEngine::Quaternion const &rotation, UnityEngine::Vector3 const &point)
    {
        float num = rotation.x * 2.0f;
        float num2 = rotation.y * 2.0f;
        float num3 = rotation.z * 2.0f;
        float num4 = rotation.x * num;
        float num5 = rotation.y * num2;
        float num6 = rotation.z * num3;
        float num7 = rotation.x * num2;
        float num8 = rotation.x * num3;
        float num9 = rotation.y * num3;
        float num10 = rotation.w * num;
        float num11 = rotation.w * num2;
        float num12 = rotation.w * num3;
        FastVector3 result;
        result.x = (1.0f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
        result.y = (num7 + num12) * point.x + (1.0f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
        result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1.0f - (num4 + num5)) * point.z;
        return result;
    }

    inline static std::string QuaternionStr(UnityEngine::Quaternion const& quaternion) {
        return "x: " + std::to_string(quaternion.x) + ", y: " + std::to_string(quaternion.y) + ", z: " + std::to_string(quaternion.z) + " w:" + std::to_string(quaternion.w);
    }

    struct FastQuaternion : public UnityEngine::Quaternion {
    public:
        // Implicit convert of quaternion
        constexpr FastQuaternion(const Quaternion& quaternion) :Quaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w) {} // x(quaternion.x), y(quaternion.y), z(quaternion.z) {}

#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
        template<Sombrero::Float T>
        constexpr FastQuaternion(const T& value) : Quaternion(value, value, value, value) {}

        template<Sombrero::HasRGBA T>
        constexpr FastQuaternion(const T& color) : Quaternion(color.x, color.y, color.z, color.a) {}
        
        template<Sombrero::HasRGB T>
        constexpr FastQuaternion(const T& color) : Quaternion(color.x, color.y, color.z, 1.0f) {}

        template<Sombrero::Has4D T>
        constexpr FastQuaternion(const T& vector) : Quaternion(vector.x, vector.y, vector.z, vector.w) {}

        template<Sombrero::Has3D T>
        constexpr FastQuaternion(const T& vector) : Quaternion(vector.x, vector.y, vector.z, 1.0f) {}
        
        template<Sombrero::Has2D T>
        constexpr FastQuaternion(const T& vector) : Quaternion(vector.x, vector.y, 0.0f, 1.0f) {}
#endif
        constexpr FastQuaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) : Quaternion(x, y, z, w) {}

        constexpr static inline FastQuaternion identity() {
            return {0.0f, 0.0f, 0.0f, 1.0f};
        }

        inline std::string toString() const {
            return QuaternionStr(*this);
        }

        constexpr FastQuaternion operator*(const FastQuaternion &b) const { return QuaternionMultiply(*this, b); }

        constexpr FastQuaternion operator*(const UnityEngine::Quaternion &b) const { return QuaternionMultiply(*this, b); }

        constexpr FastVector3 operator*(const FastVector3 &b) const { return QuaternionMultiply(*this, b); }

        constexpr FastVector3 operator*(const UnityEngine::Vector3 &b) const { return QuaternionMultiply(*this, b); }



        constexpr bool operator ==(const UnityEngine::Quaternion& lhs) {
            return lhs.x == x && lhs.y == y && lhs.z == z && lhs.w == w;
        }

        constexpr bool operator !=(const UnityEngine::Quaternion& lhs) {
            return lhs.x != x || lhs.y != y || lhs.z != z || lhs.w != w;
        }

        constexpr float& operator[](int i) {
            return (&x)[i];
        }
    };

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Quaternion) == sizeof(FastQuaternion));
#endif
}
DEFINE_IL2CPP_ARG_TYPE(Sombrero::FastQuaternion, "UnityEngine", "Quaternion");

namespace std {
    template <> 
    struct hash<Sombrero::FastQuaternion>
    {
        size_t operator()(const Sombrero::FastQuaternion & quat) const
        {
            std::hash<float> h;
            return h(quat.x) ^ h(quat.y) ^ h(quat.z) ^ h(quat.w);
        }
    };
}