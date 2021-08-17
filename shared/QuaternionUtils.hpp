#pragma once

#include "MiscUtils.hpp"
#include "Vector3Utils.hpp"

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
    }
}
#endif

namespace Sombrero {

    struct FastQuaternion;

    static UnityEngine::Quaternion QuaternionMultiply(UnityEngine::Quaternion const &lhs, UnityEngine::Quaternion const &rhs)
    {
        return UnityEngine::Quaternion(lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
    }

    static FastVector3 QuaternionMultiply(UnityEngine::Quaternion const &rotation, UnityEngine::Vector3 const &point)
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

        constexpr FastQuaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) : Quaternion(x, y, z, w) {}

        constexpr static inline FastQuaternion identity() {
            return {0.0f, 0.0f, 0.0f, 1.0f};
        }

        inline std::string toString() {
            return QuaternionStr(*this);
        }

        inline static FastQuaternion Lerp(FastQuaternion const& a, FastQuaternion const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static FastQuaternion LerpUnclamped(FastQuaternion const& a, FastQuaternion const& b, float const& t)
        {
            return FastQuaternion(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
        }

        FastQuaternion operator*(const FastQuaternion &b) const { return QuaternionMultiply(*this, b); }

        FastQuaternion operator*(const UnityEngine::Quaternion &b) const { return QuaternionMultiply(*this, b); }

        FastVector3 operator*(const FastVector3 &b) const { return QuaternionMultiply(*this, b); }

        FastVector3 operator*(const UnityEngine::Vector3 &b) const { return QuaternionMultiply(*this, b); }



        bool operator ==(const UnityEngine::Quaternion& lhs) {
            return lhs.x == x && lhs.y == y && lhs.z == z && lhs.w == w;
        }

        bool operator !=(const UnityEngine::Quaternion& lhs) {
            return lhs.x != x || lhs.y != y || lhs.z != z || lhs.w != w;
        }

        float& operator[](int i) {
            return (&x)[i];
        }
    };

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Quaternion) == sizeof(FastQuaternion));
#endif
}
DEFINE_IL2CPP_ARG_TYPE(Sombrero::FastQuaternion, "UnityEngine", "Quaternion");