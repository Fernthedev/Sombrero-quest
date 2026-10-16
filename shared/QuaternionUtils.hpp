#pragma once

#include "MiscUtils.hpp"
#include "Vector3Utils.hpp"

#include <functional>
#include <numbers>
#include <stdexcept>
#include <utility>

#include "beatsaber-hook/shared/config.hpp"
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
        
        constexpr Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}
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
        static constexpr float kEpsilon = 0.000001f;

        // Implicit convert of quaternion
        constexpr FastQuaternion(const Quaternion& quaternion) :Quaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w) {} // x(quaternion.x), y(quaternion.y), z(quaternion.z) {}

#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
        template<Sombrero::Float T>
        constexpr FastQuaternion(const T& value) : Quaternion(value, value, value, value) {}

        template<Sombrero::HasRGBA T>
        constexpr FastQuaternion(const T& color) : Quaternion(color.r, color.g, color.b, color.a) {}
        
        template<Sombrero::HasRGB T>
        constexpr FastQuaternion(const T& color) : Quaternion(color.r, color.g, color.b, 1.0f) {}

        template<Sombrero::Has4D T>
        constexpr FastQuaternion(const T& vector) : Quaternion(vector.x, vector.y, vector.z, vector.w) {}

        template<Sombrero::Has3D T>
        constexpr FastQuaternion(const T& vector) : Quaternion(vector.x, vector.y, vector.z, 1.0f) {}
        
        template<Sombrero::Has2D T>
        constexpr FastQuaternion(const T& vector) : Quaternion(vector.x, vector.y, 0.0f, 1.0f) {}
#endif
        // Like default(UnityEngine.Quaternion); use identity() for no rotation.
        constexpr FastQuaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : Quaternion(x, y, z, w) {}

        constexpr static inline FastQuaternion identity() {
            return {0.0f, 0.0f, 0.0f, 1.0f};
        }

        inline std::string toString() const {
            return QuaternionStr(*this);
        }

        constexpr void Set(float newX, float newY, float newZ, float newW) {
            x = newX;
            y = newY;
            z = newZ;
            w = newW;
        }

        constexpr static float Dot(UnityEngine::Quaternion const& a, UnityEngine::Quaternion const& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        constexpr static FastQuaternion Normalize(UnityEngine::Quaternion const& q)
        {
            float mag = std::sqrt(Dot(q, q));
            // A nonzero float dot product has sqrt >= sqrt(denorm_min), which
            // exceeds both Unity Mathf.Epsilon values (with or without FTZ).
            // Thus this matches mag < Mathf.Epsilon without an IL2CPP call.
            if (mag == 0.0f)
                return identity();

            return FastQuaternion(q.x / mag, q.y / mag, q.z / mag, q.w / mag);
        }

        constexpr void Normalize() {
            *this = Normalize(*this);
        }

        static float Angle(UnityEngine::Quaternion const& a, UnityEngine::Quaternion const& b) {
            float dot = std::abs(Dot(a, b));
            // Preserve Mathf.Min's NaN behavior as well as its upper clamp.
            dot = dot < 1.0f ? dot : 1.0f;
            constexpr float rad2Deg = 1.0f / (std::numbers::pi_v<float> * 2.0f / 360.0f);
            return IsEqualUsingDot(dot) ? 0.0f : std::acos(dot) * 2.0f * rad2Deg;
        }

        // UnityEngine's public Euler API takes degrees and applies Z, then X,
        // then Y: qY * qX * qZ. Scalar port of Unity.Mathematics.EulerZXY:
        // https://github.com/Unity-Technologies/Unity.Mathematics/blob/f53664de3dad88869a91ef8ebdbecf356e1312d5/src/Unity.Mathematics/quaternion.cs
        // See licenses/Unity.Mathematics.md for attribution.
        static FastQuaternion Euler(float x, float y, float z) {
            constexpr float deg2Rad = std::numbers::pi_v<float> * 2.0f / 360.0f;
            const float halfX = (x * deg2Rad) * 0.5f;
            const float halfY = (y * deg2Rad) * 0.5f;
            const float halfZ = (z * deg2Rad) * 0.5f;
            float sx, cx, sy, cy, sz, cz;
#ifdef __ANDROID__
            // Bionic computes each pair together, sharing argument reduction.
            ::sincosf(halfX, &sx, &cx);
            ::sincosf(halfY, &sy, &cy);
            ::sincosf(halfZ, &sz, &cz);
#else
            sx = std::sin(halfX); cx = std::cos(halfX);
            sy = std::sin(halfY); cy = std::cos(halfY);
            sz = std::sin(halfZ); cz = std::cos(halfZ);
#endif
            // A product of unit axis rotations needs no normalization pass.
            return {
                sx * cy * cz + sy * sz * cx,
                sy * cx * cz - sx * sz * cy,
                sz * cx * cy - sx * sy * cz,
                cx * cy * cz + sy * sz * sx
            };
        }

        static FastQuaternion Euler(UnityEngine::Vector3 const& euler) {
            return Euler(euler.x, euler.y, euler.z);
        }

        // Interpolation and the other native Unity algorithms remain inherited
        // when HAS_CODEGEN is enabled.

        constexpr auto get_normalized() const
        {
            return FastQuaternion::Normalize(*this);
        }

        constexpr FastQuaternion operator*(const FastQuaternion &b) const { return QuaternionMultiply(*this, b); }

        constexpr FastQuaternion operator*(const UnityEngine::Quaternion &b) const { return QuaternionMultiply(*this, b); }

        constexpr FastVector3 operator*(const FastVector3 &b) const { return QuaternionMultiply(*this, b); }

        constexpr FastVector3 operator*(const UnityEngine::Vector3 &b) const { return QuaternionMultiply(*this, b); }



        constexpr bool operator ==(const UnityEngine::Quaternion& rhs) const {
            return IsEqualUsingDot(Dot(*this, rhs));
        }

        constexpr bool operator !=(const UnityEngine::Quaternion& rhs) const {
            return !(*this == rhs);
        }

        // Unity's Equals is exact component equality (including NaN.Equals(NaN)),
        // distinct from its approximate rotation operator==.
        constexpr bool Equals(const UnityEngine::Quaternion& rhs) const {
            return FloatEquals(x, rhs.x) && FloatEquals(y, rhs.y)
                && FloatEquals(z, rhs.z) && FloatEquals(w, rhs.w);
        }

#ifdef HAS_CODEGEN
        bool Equals(::System::Object* rhs) const {
            auto* object = reinterpret_cast<Il2CppObject*>(rhs);
            return object && object->klass == ::i2c::class_of<UnityEngine::Quaternion>()
                && Equals(::i2c::from_object<UnityEngine::Quaternion, true>(object));
        }
        constexpr bool Equals(::by_ref<const UnityEngine::Quaternion> rhs) const { return Equals(*rhs); }
#endif

        constexpr float& operator[](int i) {
            switch (i) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                default: throw std::out_of_range("Invalid Quaternion index!");
            }
        }

        constexpr const float& operator[](int i) const {
            switch (i) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                default: throw std::out_of_range("Invalid Quaternion index!");
            }
        }

    private:
        constexpr static bool IsEqualUsingDot(float dot) {
            return dot > 1.0f - kEpsilon;
        }

        constexpr static bool FloatEquals(float a, float b) {
            return a == b || (a != a && b != b);
        }
    };

#ifdef HAS_CODEGEN
    static_assert(sizeof(UnityEngine::Quaternion) == sizeof(FastQuaternion));
#endif
}
DEFINE_IL2CPP_CLASS(Sombrero::FastQuaternion, "UnityEngine", "Quaternion");
MARK_VAL_T(Sombrero::FastQuaternion);

namespace Sombrero {
    template <> 
    struct ExactHash<FastQuaternion>
    {
        size_t operator()(const Sombrero::FastQuaternion & quat) const noexcept
        {
            // Canonicalize NaN payloads to agree with Equals. std::hash<float>
            // already gives +0 and -0 the same hash. Hash values are C++-local.
            auto h = detail::FloatKeyHash;
            return h(quat.x) ^ (h(quat.y) << 2) ^ (h(quat.z) >> 2) ^ (h(quat.w) >> 1);
        }
    };

    // Exact key comparison is separate from the approximate rotation operator.
    template <>
    struct ExactEqual<FastQuaternion>
    {
        constexpr bool operator()(const Sombrero::FastQuaternion& a, const Sombrero::FastQuaternion& b) const noexcept {
            return a.Equals(b);
        }
    };
}
