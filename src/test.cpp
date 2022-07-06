// Include headers so CLion doesn't cry

#include "Vector3Utils.hpp"
#include "Vector2Utils.hpp"
#include "QuaternionUtils.hpp"
#include "ColorUtils.hpp"
#include "HSBColor.hpp"
#include "RandomUtils.hpp"
#include "linq.hpp"
#include "linq_functional.hpp"

int main() {
    Sombrero::FastVector3 vec3;
    Sombrero::vector3Str(vec3);

    Sombrero::FastVector2 vec2;
    Sombrero::vector2Str(vec2);

    Sombrero::HSBColor();

    // test concepts to see if we are allowed to assign a vector3 to a color

    static_assert(Sombrero::Clamp01(2.0f) == 1.0f);
    static_assert(Sombrero::Clamp01(0.8f) == 0.8f);
    static_assert(Sombrero::Clamp01(-1.2f) == 0.0f);

#ifdef USE_SOMBRERO_IMPLICIT_CONVERSIONS
    Sombrero::FastColor color = vec3;
    Sombrero::FastVector2 vec2_2 = color;

    Sombrero::Lerp(vec2, vec2_2, 0.8f);

    Sombrero::FastVector3 vec3_2;
    Sombrero::FastQuaternion quat;
    Sombrero::FastQuaternion quat2;
    Sombrero::Lerp(quat, quat2, 0.5f);

    vec3_2 += vec3_2;

    vec3_2 += 1.0f;
    vec3_2 += quat;
    vec3_2 += vec2;
#endif
    constexpr float val = Sombrero::power(2.0f, 2);
    constexpr float val2 = Sombrero::power(2.0f, -1);
    constexpr float val3 = Sombrero::power(3, -2);
    constexpr float val4 = Sombrero::power(5, -3);
    static_assert(val == 4.0f);
    static_assert(val2 == 0.5f);
    static_assert(val3 == 1.0f / 9.0f);
    static_assert(val4 == 1.0f / 125.0f);

    using namespace Sombrero::Linq;
    ArrayW<int> a(5);
    for (auto item : Select(a, [](auto& v) {return float(v);})) {
        // iterate the selected floats
    }
    // Show that a captureful lambda plays nice
    for (auto item : Where(a, [&](auto& v) {a[0] = 3; return v == 0;})) {
        // iterate all the items that are exactly 0
    }
    ArrayW<int> x(ToArray(a));
    auto coll = a | Functional::Where([](int x) {return x > 3;})
                | Functional::Select([](int x) {return bool(x);})
                | Functional::ToArray();
    static_assert(std::is_same_v<decltype(coll), ArrayW<bool>>);
    for (auto item : coll) {
        static_assert(std::is_same_v<decltype(item), bool>);
        // iterate all the items that are greater than 3
    }
}