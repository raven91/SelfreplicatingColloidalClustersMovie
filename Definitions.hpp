//
// Created by Nikita Kruk on 01.03.20.
//

#ifndef SELFREPLICATINGCOLLOIDALCLUSTERSMOVIE_DEFINITIONS_HPP
#define SELFREPLICATINGCOLLOIDALCLUSTERSMOVIE_DEFINITIONS_HPP

#include <cmath>
#include <array>

typedef float Real; // type of the simulation data

enum class ParticleRepresentationType
{
  kPointMass, kSphere
};

inline float ModWrap0Denom(float numerator, float denominator)
{
  return numerator - denominator * std::floorf(numerator / denominator);
}

inline void GetSphericalCoordinate(float r, float phi, float theta, std::array<float, 3> &pt)
{
  pt[0] = r * std::sinf(theta) * std::cosf(phi);
  pt[1] = r * std::sinf(theta) * std::sinf(phi);
  pt[2] = r * std::cosf(theta);
}

#endif //SELFREPLICATINGCOLLOIDALCLUSTERSMOVIE_DEFINITIONS_HPP
