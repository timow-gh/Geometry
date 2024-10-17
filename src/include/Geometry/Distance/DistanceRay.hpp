#ifndef GEOMETRY_DISTANCERAY_HPP
#define GEOMETRY_DISTANCERAY_HPP

#include "Geometry/ClosetPointOnLine.hpp"
#include "Geometry/Distance/DistanceLine.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
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

  return linal::length(distanceVec);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Ray<T, D>& ray) noexcept
{
  return distance(ray, vec);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& line, const Ray<T, D>& ray) noexcept
{
  const linal::vec<T, D> lineSource = line.get_origin();
  const linal::vec<T, D> linaDir = line.get_direction();
  const linal::vec<T, D> raySource = ray.get_origin();
  const linal::vec<T, D> rayDir = ray.get_direction();

  if (auto params = details::closest_point_on_line_parameters<T>(lineSource, linaDir, raySource, rayDir))
  {
    if (params->s < 0)
    {
      return distance(raySource, line);
    }

    linal::vec<T, D> linePoint = lineSource + (params->t * linaDir);
    linal::vec<T, D> rayPoint = raySource + (params->s * rayDir);
    return linal::length(linePoint - rayPoint);
  }

  return linal::length(lineSource - raySource);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Ray<T, D>& ray, const Line<T, D>& line) noexcept
{
  return distance(line, ray);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCERAY_HPP
