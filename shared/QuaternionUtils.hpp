#pragma once

#include "MiscUtils.hpp"
#include "Vector3Utils.hpp"

#ifdef HAS_CODEGEN
#include "UnityEngine/Quaternion.hpp"
#endif


namespace Sombrero {

    struct FastQuaternion;

    template<typename T>
    concept QuaternionDerive = requires() {
#ifdef HAS_CODEGEN
        std::is_base_of<UnityEngine::Quaternion, T>::value;
#else
        std::is_base_of<FastQuaternion, T>::value;
#endif
        !std::is_pointer<T>::value;
    };

    template<QuaternionDerive T>
    inline static std::string QuaternionStr(T const& quaternion) {
        return "x: " + std::to_string(quaternion.x) + ", y: " + std::to_string(quaternion.y) + ", z: " + std::to_string(quaternion.z) + " w:" + std::to_string(quaternion.w);
    }

    template<QuaternionDerive T>
    static FastVector3 QuaternionMultiply(T const& rotation, FastVector3 point) {
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

    template<QuaternionDerive T>
    static T QuaternionMultiply(T const& lhs, T const& rhs)
    {
        return T(lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
    }

#ifdef HAS_CODEGEN
    struct FastQuaternion : public UnityEngine::Quaternion {
#else
    struct FastQuaternion {
        float x, y, z, w;
#endif
    public:
#ifdef HAS_CODEGEN
        // Implicit convert of quaternion
        FastQuaternion(const Quaternion& quaternion) :Quaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w) {} // x(quaternion.x), y(quaternion.y), z(quaternion.z) {}

        FastQuaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : Quaternion(x, y, z, w) {}
#else
        FastQuaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}
#endif
        static constexpr const identity = {0.0f, 0.0f, 0.0f, 1.0f};

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

        FastVector3 operator*(const FastVector3 &b) const { return QuaternionMultiply(*this, b); }

        template<QuaternionDerive T>
        bool operator ==(const T& lhs) {
            return lhs.x == x && lhs.y == y && lhs.z == z && lhs.w == w;
        }

        template<QuaternionDerive T>
        inline bool operator !=(const T& lhs) {
            return !(this == lhs);
        }

        float& operator[](int i) {
            return (&x[i]);
        }
    };

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Quaternion) == sizeof(FastQuaternion));
#else
    DEFINE_IL2CPP_ARG_TYPE(FastQuaternion, "UnityEngine", "Quaternion");
#endif
}