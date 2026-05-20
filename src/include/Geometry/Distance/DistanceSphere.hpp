#ifndef GEOMETRY_DISTANCESPHERE_H
#define GEOMETRY_DISTANCESPHERE_H

#include "Geometry/Sphere.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD T signed_distance(const linal::vec3<T>& vec, const Sphere<T>& sphere) noexcept
{
  return linal::length(linal::vec3<T>{vec - sphere.get_origin()}) - sphere.get_radius();
}

template <typename T>
GEO_NODISCARD T distance(const linal::vec3<T>& vec, const Sphere<T>& sphere) noexcept
{
  return std::abs(signed_distance(vec, sphere));
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCESPHERE_H
