#ifndef GEOMETRY_DISTANCESEGMENT_HPP
#define GEOMETRY_DISTANCESEGMENT_HPP

#include "Geometry/ClosetPointOnLine.hpp"
#include "Geometry/Distance/DistanceLine.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

namespace details
{

template <typename T, std::uint8_t D>
linal::vec<T, D> calc_segment_point(const Segment<T, D>& segment, T parameter)
{
  if (parameter < 0)
  {
    return segment.get_source();
  }
  else if (parameter > 1)
  {
    return segment.get_target();
  }
  else
  {
    return segment.get_source() + parameter * (segment.get_target() - segment.get_source());
  }
}

template <typename T, std::uint8_t D>
linal::vec<T, D> calc_ray_point(const Ray<T, D>& ray, T parameter)
{
  if (parameter < 0)
  {
    return ray.get_origin();
  }
  else
  {
    return ray.get_origin() + parameter * ray.get_direction();
  }
}

} // namespace details

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Segment<T, D>& segment, linal::vec<T, D> vec) noexcept
{
  linal::vec<T, D> source = segment.get_source();
  linal::vec<T, D> ps = vec - source;
  linal::vec<T, D> dir = segment.get_target() - source;
  T parameter = linal::dot(ps, dir) / linal::length(dir);
  T projParameter = std::min(std::max(parameter, T(0.0)), T(1.0));
  linal::vec<T, D> projVec{source + projParameter * dir};
  return linal::length(linal::vec<T, D>{projVec - vec});
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Segment<T, D>& segment) noexcept
{
  return distance(segment, vec);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& line, const Segment<T, D>& segment) noexcept
{
  return distance(segment, line);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Segment<T, D>& lhs, const Segment<T, D>& rhs) noexcept
{
  const linal::vec<T, D> lhsSource = lhs.get_source();
  const linal::vec<T, D> lhsDir = lhs.get_target() - lhsSource;
  const linal::vec<T, D> rhsSource = rhs.get_origin();
  const linal::vec<T, D> rhsDir = rhs.get_target() - rhsSource;

  auto params = details::closest_point_on_line_parameters<T>(lhsSource, lhsDir, rhsSource, rhsDir);

  if (!params)
  {
    // parallel lines.
    GEO_ASSERT(false);
    return -1;
  }

  linal::vec<T, D> lhsPoint = details::calc_segment_point(lhs, params->t);
  linal::vec<T, D> rhsPoint = details::calc_segment_point(rhs, params->s);

  return linal::length(lhsPoint - rhsPoint);
}

/** @brief Calculates the distance between a line and a segment. Can't handle parallel or collinear elements. */
template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Segment<T, D>& segment, const Ray<T, D>& ray) noexcept
{
  const linal::vec<T, D> segSource = segment.get_source();
  const linal::vec<T, D> segDir = segment.get_target() - segSource;
  const linal::vec<T, D> raySource = ray.get_origin();
  const linal::vec<T, D> rayDir = ray.get_direction();

  auto params = details::closest_point_on_line_parameters<T>(segSource, segDir, raySource, rayDir);

  if (!params)
  {
    linal::vec<T, D> source = segment.get_source();
    linal::vec<T, D> diffVec = raySource - source;
    linal::vec<T, D> dir = segment.get_target() - source;
    T parameter = linal::dot(diffVec, dir) / linal::length(dir);

    // parallel lines.
    const double lineDist = distance(Line<T, D>{segSource, segDir}, raySource);
    if (!linal::isEq(lineDist, 0.0))
    {
      if (linal::isGreaterEq(parameter, T(0)) && linal::isLessEq(parameter, T(1)))
      {
        return lineDist;
      }

      if (linal::isLess(parameter, T(0)))
      {
        if (linal::dot(rayDir, dir) < 0)
        {
          // Ray is pointing away from the segment.
          return linal::length(diffVec);
        }
        // Ray is pointing towards the segment. The line is the closest element.
        return lineDist;
      }

      if (linal::isGreater(parameter, T(1)))
      {
        if (linal::dot(rayDir, dir) > 0)
        {
          // Ray is pointing away from the segment.
          return linal::length(raySource - segment.get_target());
        }
        // Ray is pointing towards the segment. The line is the closest element.
        return lineDist;
      }
    }

    // Collinear lines.
    if (linal::isGreaterEq(parameter, T(0)) && linal::isLessEq(parameter, T(1)))
    {
      return 0;
    }

    if (linal::isLess(parameter, T(0)))
    {
      if (linal::dot(rayDir, dir) < 0)
      {
        // Ray is pointing away from the segment.
        return linal::length(diffVec);
      }
      // Ray is pointing towards the segment.
      return 0;
    }

    if (linal::isGreater(parameter, T(1)))
    {
      if (linal::dot(rayDir, dir) > 0)
      {
        // Ray is pointing away from the segment.
        return linal::length(raySource - segment.get_target());
      }
      // Ray is pointing towards the segment.
      return 0;
    }
  }

  linal::vec<T, D> segPoint = details::calc_segment_point(segment, params->t);
  linal::vec<T, D> rayPoint = details::calc_ray_point(ray, params->s);

  return linal::length(segPoint - rayPoint);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Ray<T, D>& ray, const Segment<T, D>& segment) noexcept
{
  return distance(segment, ray);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCESEGMENT_HPP
