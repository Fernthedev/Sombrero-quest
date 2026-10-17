#pragma once

#include "MiscUtils.hpp"
#include "Concepts.hpp"

#include <utility>
#include <stdexcept>

#include "beatsaber-hook/shared/config.hpp"
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
        
        constexpr Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    };
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
        static constexpr float kEpsilon = 0.00001f;

        // Implicit convert of vector
        constexpr FastVector2(const Vector2& vector) : Vector2(vector.x, vector.y) {}
        constexpr FastVector2(float x = 0.0f, float y = 0.0f) : Vector2(x, y) {}

#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
        template<Sombrero::Float T>
        constexpr FastVector2(const T& value) : Vector2(value, value) {}

        template<Sombrero::HasRGBA T>
        constexpr FastVector2(const T& color) : Vector2(color.r, color.g) {}
        
        template<Sombrero::HasRGB T>
        constexpr FastVector2(const T& color) : Vector2(color.r, color.g) {}

        template<Sombrero::Has4D T>
        constexpr FastVector2(const T& vector) : Vector2(vector.x, vector.y) {}

        template<Sombrero::Has3D T>
        constexpr FastVector2(const T& vector) : Vector2(vector.x, vector.y) {}

        template<Sombrero::Has2D T>
        constexpr FastVector2(const T& vector) : Vector2(vector.x, vector.y) {}
#endif
        CONSTEXPR_GETTER(one, {1.0f, 1.0f})
        CONSTEXPR_GETTER(zero, {0.0f, 0.0f})

        CONSTEXPR_GETTER(down, {0.0f, -1.0f})
        CONSTEXPR_GETTER(up, {0.0f, 1.0f})
        CONSTEXPR_GETTER(left, {-1.0f, 0.0f})
        CONSTEXPR_GETTER(right, {1.0f, 0.0f})

        inline std::string toString() const {
            return vector2Str(*this);
        }

        inline static FastVector2 Lerp(FastVector2 const& a, FastVector2 const& b, float const& t)
        {
            return LerpUnclamped(a, b, Clamp01(t));
        }

        static constexpr FastVector2 LerpUnclamped(FastVector2 const& a, FastVector2 const& b, float const& t)
        {
            return FastVector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        constexpr float Magnitude() const {
            return ::Sombrero::sqroot((x * x) + (y * y));
        }

        constexpr float Distance(const FastVector2& b) const {
            float dx = x - b.x;
            float dy = y - b.y;
            return ::Sombrero::sqroot(dx * dx + dy * dy);
        }

        static constexpr FastVector2 Normalize(const FastVector2& vec) {
            float magnitude = vec.Magnitude();
            return magnitude > kEpsilon ? vec / magnitude : zero();
        }

        constexpr FastVector2 get_normalized() const {
            return Normalize(*this);
        }

        constexpr void Normalize() {
            NormalizeFast();
        }

        // In case codegen method takes over
        constexpr void NormalizeFast() {
            *this = Normalize(*this);
        }

#define operatorOverload(name, operatore) \
        constexpr FastVector2 operator operatore(const FastVector2& b) const { \
            return FastVector2(this->x operatore b.x, this->y operatore b.y); \
        }                                \
        constexpr FastVector2 operator operatore(const UnityEngine::Vector2& b) const { \
            return FastVector2(this->x operatore b.x, this->y operatore b.y); \
        }                                \
        constexpr FastVector2 operator operatore(float const& b) const { \
            return FastVector2(this->x operatore b, this->y operatore b); \
        }                                 \
        constexpr FastVector2& operator operatore##=(float const& bb) {  \
            x operatore##= bb;                       \
            y operatore##= bb;                        \
            return *this; \
        }                                 \
        constexpr FastVector2& operator operatore##=(const FastVector2& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            return *this; \
        } \
        constexpr FastVector2& operator operatore##=(const UnityEngine::Vector2& bb) {  \
            x operatore##= bb.x;                       \
            y operatore##= bb.y;                        \
            return *this; \
        }


        operatorOverload(add, +)
        operatorOverload(subtract, -)
        operatorOverload(multiply, *)
        operatorOverload(divide, /)

#undef operatorOverload

        constexpr FastVector2 operator-() const {
            return FastVector2(-x, -y);
        }


        constexpr bool operator ==(const UnityEngine::Vector2& lhs) const {
            const float dx = x - lhs.x, dy = y - lhs.y;
            return dx * dx + dy * dy < kEpsilon * kEpsilon;
        }

        constexpr bool operator !=(const UnityEngine::Vector2& lhs) const {
            return !(*this == lhs);
        }

        constexpr bool Equals(const UnityEngine::Vector2& rhs) const {
            return x == rhs.x && y == rhs.y;
        }

#ifdef HAS_CODEGEN
        // Keep generated overloads available without importing the by-value
        // overload, which would compete with the native const-reference path.
        bool Equals(::System::Object* rhs) const {
            auto* object = reinterpret_cast<Il2CppObject*>(rhs);
            return object && object->klass == ::i2c::class_of<UnityEngine::Vector2>()
                && Equals(::i2c::from_object<UnityEngine::Vector2, true>(object));
        }
        constexpr bool Equals(::by_ref<const UnityEngine::Vector2> rhs) const { return Equals(*rhs); }
#endif

        constexpr float& operator[](int i) {
            switch (i) {
                case 0: return x;
                case 1: return y;
                default: throw std::out_of_range("Invalid Vector2 index!");
            }
        }

        constexpr const float& operator[](int i) const {
            switch (i) {
                case 0: return x;
                case 1: return y;
                default: throw std::out_of_range("Invalid Vector2 index!");
            }
        }
    };

#define operatorOverload(name, operator) \
    constexpr static FastVector2 vector2##name(UnityEngine::Vector2 const &a, UnityEngine::Vector2 const &b) \
    { \
        return FastVector2(a.x operator b.x, a.y operator b.y); \
    } \
    constexpr static FastVector2 vector2##name (UnityEngine::Vector2 const& a, float const& b) { \
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
DEFINE_IL2CPP_CLASS(Sombrero::FastVector2, "UnityEngine", "Vector2");
MARK_VAL_T(Sombrero::FastVector2);
#undef CONSTEXPR_GETTER

namespace Sombrero {
    template <> 
    struct ExactHash<FastVector2>
    {
        size_t operator()(const Sombrero::FastVector2 & v) const noexcept
        {
            auto h = Sombrero::detail::FloatKeyHash;
            return h(v.x) ^ h(v.y);
        }
    };

    template <>
    struct ExactEqual<FastVector2> {
        constexpr bool operator()(const Sombrero::FastVector2& a, const Sombrero::FastVector2& b) const noexcept {
            return Sombrero::detail::FloatKeyEqual(a.x, b.x) && Sombrero::detail::FloatKeyEqual(a.y, b.y);
        }
    };
}
