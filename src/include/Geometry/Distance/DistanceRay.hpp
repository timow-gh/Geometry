#ifndef GEOMETRY_DISTANCERAY_HPP
#define GEOMETRY_DISTANCERAY_HPP

#include <Geometry/Ray.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD constexpr T distance(const Ray<T, D>& ray, linal::vec<T, D> vec) noexcept
{
  linal::vec<T, D> vecO{vec - ray.get_origin()};
  const linal::vec<T, D> rayDirection = ray.get_direction();

  const T dotProduct = linal::dot(vecO, rayDirection);
  linal::vec<T, D> distanceVec;
  if (linal::isGreater(dotProduct, T(0)))
  {
    distanceVec = linal::rejection(vecO, rayDirection);
  }
  else
  {
    distanceVec = ray.get_origin() - vec;
  }

  return linal::norm2(distanceVec);
}

template <typename T, std::size_t D>
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Ray<T, D>& ray) noexcept
{
  return distance(ray, vec);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCERAY_HPP
