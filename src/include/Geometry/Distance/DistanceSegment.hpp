#ifndef GEOMETRY_DISTANCESEGMENT_HPP
#define GEOMETRY_DISTANCESEGMENT_HPP

#include "Geometry/ClosetPointOnLine.hpp"
#include "Geometry/Distance/DistanceLine.hpp"
#include "Geometry/Distance/DistanceRay.hpp"
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

/** @brief Calculate the point on a segment.
 *
 * If the parameter is less than 0, the source point is returned.
 * If the parameter is greater than 1, the target point is returned.
 * Otherwise, the point on the segment is calculated.
 */
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

/** @brief Calculate the point on a ray.
 *
 * If the parameter is less than 0, the origin point is returned.
 * Otherwise, the point on the ray is calculated.
 */
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
  T dirLength = linal::length(dir);
  if (linal::isZero(dirLength, linal::eps<T>::value))
  {
    return linal::length(ps); // degenerate segment: distance to the single point
  }
  T parameter = linal::dot(ps, dir) / linal::dot(dir, dir);
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
  const linal::vec<T, D> rhsSource = rhs.get_source();
  const linal::vec<T, D> rhsDir = rhs.get_target() - rhsSource;

  auto params = details::closest_point_on_line_parameters<T>(lhsSource, lhsDir, rhsSource, rhsDir);

  if (!params)
  {
    // Parallel (or collinear) lines: closest_point_on_line_parameters cannot produce a unique
    // solution. Both segments are bounded, so the minimum distance is either 0 (if the segments'
    // lines are coincident and their projected parameter ranges overlap) or the minimum distance
    // from one of the four endpoints to the other segment -- the closest points between two
    // parallel/collinear segments always include at least one segment's endpoint.
    const linal::vec<T, D> rhsTarget = rhs.get_target();
    const T lineDist = distance(Line<T, D>{lhsSource, lhsDir}, rhsSource);

    if (linal::isZero(lineDist, linal::eps<T>::value))
    {
      // Collinear lines: check whether the segments' parameter ranges (projected onto the shared
      // line) overlap; if so the distance is 0.
      const T dirDot = linal::dot(lhsDir, lhsDir);
      if (!linal::isZero(dirDot, linal::eps<T>::value))
      {
        const T tSource = T(0);
        const T tTarget = T(1);
        T sSource = linal::dot(linal::vec<T, D>{rhsSource - lhsSource}, lhsDir) / dirDot;
        T sTarget = linal::dot(linal::vec<T, D>{rhsTarget - lhsSource}, lhsDir) / dirDot;
        if (sSource > sTarget)
        {
          std::swap(sSource, sTarget);
        }
        const T lo = std::max(tSource, sSource);
        const T hi = std::min(tTarget, sTarget);
        if (linal::isLessEq(lo, hi))
        {
          return 0;
        }
      }
    }

    const T d1 = distance(lhs, rhsSource);
    const T d2 = distance(lhs, rhsTarget);
    const T d3 = distance(rhs, lhsSource);
    const T d4 = distance(rhs, lhs.get_target());
    return std::min(std::min(d1, d2), std::min(d3, d4));
  }

  linal::vec<T, D> lhsPoint = details::calc_segment_point(lhs, params->t);
  linal::vec<T, D> rhsPoint = details::calc_segment_point(rhs, params->s);

  return linal::length(lhsPoint - rhsPoint);
}

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
    T dirLength = linal::length(dir);
    if (linal::isZero(dirLength, linal::eps<T>::value))
    {
      // Degenerate segment: the segment is a single point, so this reduces to point-to-ray distance.
      return distance(source, ray);
    }
    T parameter = linal::dot(diffVec, dir) / linal::dot(dir, dir);

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
