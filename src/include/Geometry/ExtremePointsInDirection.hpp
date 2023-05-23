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

template <typename TFloat, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR MinMax<TFloat> extremePointsAlongDirection(linal::vec<TFloat, D> dir,
                                                                       const linal::vecvector<TFloat, D>& points) GEO_NOEXCEPT
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
