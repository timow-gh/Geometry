#ifndef GEOMETRY_DISTANCERAY_HPP
#define GEOMETRY_DISTANCERAY_HPP

#include <Geometry/Ray.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <linal/utils/eps.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Ray<T, D>& ray, linal::vec<T, D> vec)
{
  linal::vec<T, D> vecO{vec - ray.getOrigin()};
  const linal::vec<T, D> rayDirection = ray.getDirection();

  const T dotProduct = linal::dot(vecO, rayDirection);
  linal::vec<T, D> distanceVec;
  if (linal::isGreater(dotProduct, T(0)))
    distanceVec = linal::rejection(vecO, rayDirection);
  else
    distanceVec = ray.getOrigin() - vec;

  return linal::norm2(distanceVec);
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::vec<T, D> vec, const Ray<T, D>& ray)
{
  return distance(ray, vec);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCERAY_HPP
