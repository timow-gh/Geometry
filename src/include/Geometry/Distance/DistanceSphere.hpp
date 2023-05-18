#ifndef GEOMETRY_DISTANCESPHERE_H
#define GEOMETRY_DISTANCESPHERE_H

#include <Geometry/Sphere.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD T signedDistance(const linal::vec3<T>& vec, const Sphere<T>& sphere)
{
  return linal::norm2(linal::vec3<T>{vec - sphere.getOrigin()}) - sphere.getRadius();
}

template <typename T>
GEO_NODISCARD T distance(const linal::vec3<T>& vec, const Sphere<T>& sphere)
{
  return std::abs(signedDistance(vec, sphere));
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCESPHERE_H
