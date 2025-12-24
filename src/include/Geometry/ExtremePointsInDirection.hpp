#ifndef GEOMETRY_EXTREMEPOINTSINDIRECTION_HPP
#define GEOMETRY_EXTREMEPOINTSINDIRECTION_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>
#include <span>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <vector>

namespace Geometry
{

template <typename TFloat>
struct MinMax
{
  TFloat min{0.0};
  TFloat max{0.0};

  static MinMax<TFloat> merge(const MinMax<TFloat>& a, const MinMax<TFloat>& b) noexcept
  {
    return {std::min(a.min, b.min), std::max(a.max, b.max)};
  }
};

namespace details
{

template <typename TFloat, std::uint8_t D>
void calc_min_max(TFloat& minVal, TFloat& maxVal, linal::vec<TFloat, D> dir, linal::vec<TFloat, D> point)
{
  TFloat dist = linal::dot(point, dir);
  if (dist < minVal)
  {
    minVal = dist;
  }
  if (dist > maxVal)
  {
    maxVal = dist;
  }
}

} // namespace details

/* @brief Returns extreme points of a set of points along a given direction.
 *
 * @param dir Direction along which to find extreme points.
 * @param begin Iterator to the beginning of the points.
 * @param end Iterator to the end of the points.
 * @return Min and max values.
 */
template <typename TFloat, std::uint8_t D, typename TPointIter>
GEO_NODISCARD constexpr MinMax<TFloat> extreme_points_along_direction(linal::vec<TFloat, D> dir, TPointIter begin, TPointIter end) noexcept
{
  GEO_ASSERT(begin != end);
  GEO_ASSERT(linal::isEq(linal::length(dir), TFloat{1}));

  MinMax<TFloat> result;

  for (auto it = begin; it != end; ++it)
  {
    details::calc_min_max(result.min, result.max, dir, *it);
  }

  return result;
}

/** \copydoc Geometry::extreme_points_along_direction(linal::vec<TFloat, D>, TPointIter, TPointIter) */
template <typename TFloat, std::uint8_t D>
GEO_NODISCARD constexpr MinMax<TFloat> extreme_points_along_direction(linal::vec<TFloat, D> dir,
                                                                      const std::vector<linal::vec<TFloat, D>>& points) noexcept
{
  return extreme_points_along_direction<TFloat, D>(dir, points.begin(), points.end());
}

/** \copydoc Geometry::extreme_points_along_direction(linal::vec<TFloat, D>, TPointIter, TPointIter) */
template <typename TFloat, std::uint8_t D>
GEO_NODISCARD constexpr MinMax<TFloat> extreme_points_along_direction(linal::vec<TFloat, D> dir, std::span<const TFloat> points) noexcept
{
  GEO_ASSERT(points.size() % D == 0);
  GEO_ASSERT(linal::isEq(linal::length(dir), TFloat{1}));

  MinMax<TFloat> result;

  for (std::size_t i = 0; i < points.size(); i += D)
  {
    linal::vec<TFloat, D> point;
    for (std::uint8_t d = 0; d < D; ++d)
    {
      point[d] = points[i + d];
    }
    details::calc_min_max(result.min, result.max, dir, point);
  }

  return result;
}

} // namespace Geometry

#endif // GEOMETRY_EXTREMEPOINTSINDIRECTION_HPP
