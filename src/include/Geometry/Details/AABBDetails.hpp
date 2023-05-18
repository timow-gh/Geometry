#ifndef GEOMETRY_AABBDETAILS_HPP
#define GEOMETRY_AABBDETAILS_HPP

#include <Geometry/AABB.hpp>
#include <cstdint>
#include <linal/Containers.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{

namespace details
{

template <typename TFloat>
struct MinMax
{
  TFloat min{0.0};
  TFloat max{0.0};
};

template <typename TFloat, std::size_t D>
MinMax<TFloat> extremePointsAlongDirection(linal::Vec<TFloat, D> dir, const linal::VecVector<TFloat, D>& points)
{
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
  return {minDist, maxDist};
}

} // namespace details
} // namespace Geometry

#endif // GEOMETRY_AABBDETAILS_HPP
