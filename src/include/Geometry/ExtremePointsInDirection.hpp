#ifndef GEOMETRY_EXTREMEPOINTSINDIRECTION_HPP
#define GEOMETRY_EXTREMEPOINTSINDIRECTION_HPP

#include <Geometry/Utils/Assert.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <cstdint>
#include <linal/containers.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename TFloat>
struct MinMax
{
  TFloat min{0.0};
  TFloat max{0.0};
};

/* @brief Returns extreme points of a set of points along a given direction.
 *
 * @param dir Direction along which to find extreme points.
 * @param points Set of points.
 * @return MinMax structure with min and max values.
 */
template <typename TFloat, std::size_t D>
GEO_NODISCARD constexpr MinMax<TFloat> extreme_points_along_direction(linal::vec<TFloat, D> dir,
                                                                      const linal::vecvector<TFloat, D>& points) noexcept
{
  GEO_ASSERT(!points.empty());
  GEO_ASSERT(linal::isEq(linal::norm2(dir), TFloat{1}));

  TFloat minDist = std::numeric_limits<TFloat>::max();
  TFloat maxDist = std::numeric_limits<TFloat>::lowest();

  for (auto vec: points)
  {
    TFloat dist = linal::dot(vec, dir);
    if (dist < minDist)
    {
      minDist = dist;
    }
    if (dist > maxDist)
    {
      maxDist = dist;
    }
  }

  return MinMax<TFloat>{minDist, maxDist};
}

} // namespace Geometry

#endif // GEOMETRY_EXTREMEPOINTSINDIRECTION_HPP
