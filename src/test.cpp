// Include headers so CLion doesn't cry

#include "Vector3Utils.hpp"
#include "Vector2Utils.hpp"
#include "QuaternionUtils.hpp"
#include "ColorUtils.hpp"
#include "HSBColor.hpp"
#include "RandomUtils.hpp"

int main() {
    Sombrero::FastVector3 vec3;
    Sombrero::vector3Str(vec3);

    Sombrero::FastVector2 vec2;
    Sombrero::vector2Str(vec2);

    Sombrero::HSBColor();
}