#pragma once

#include <random>

namespace Sombrero {
    class RandomFast {
        public:
        // https://stackoverflow.com/a/686373/9816000
        // https://www.cplusplus.com/reference/random/uniform_real_distribution/
        // https://www.delftstack.com/howto/cpp/how-to-generate-random-float-number-in-cpp/
        static float randomNumber(float min, float max = 1.0f) {
            static std::random_device randomDevice;
            static std::default_random_engine eng(randomDevice());
            if (min > max) {
                auto temp = min;
                min = max;
                max = temp;
            }

            std::uniform_real_distribution<float> distr(min, max);

            return distr(eng);
        }

        /// Returns between 0.0f and 1.0f
        static float randomNumber() {
            static std::random_device randomDevice;
            static std::default_random_engine eng(randomDevice());
            static std::uniform_real_distribution<float> distr(0.0f, 1.0f);

            return distr(eng);
        }
    };
}