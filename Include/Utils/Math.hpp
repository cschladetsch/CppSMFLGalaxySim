#pragma once

#include <glm/glm.hpp>
#include <numbers>

namespace Utils {

constexpr float PI = std::numbers::pi_v<float>;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = PI * 0.5f;

inline float RadToDeg(float radians) { return radians * (180.0f / PI); }

inline float DegToRad(float degrees) { return degrees * (PI / 180.0f); }

template <typename T> inline T Lerp(const T &a, const T &b, float t) {
  return a + (b - a) * t;
}

template <typename T>
inline T Clamp(const T &value, const T &min, const T &max) {
  return glm::clamp(value, min, max);
}

} // namespace Utils