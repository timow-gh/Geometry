#ifndef GEOMETRY_INTERSECTIONAABB_HPP
#define GEOMETRY_INTERSECTIONAABB_HPP

#include <Geometry/AABB.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR bool isIntersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs) GEO_NOEXCEPT
{
  auto lhsMax = lhs.get_max();
  auto lhsMin = lhs.get_min();
  auto rhsMax = rhs.get_max();
  auto rhsMin = rhs.get_min();

  for (std::size_t i = 0; i < D; ++i)
  {
    if (lhsMax[i] < rhsMin[i] || lhsMin[i] > rhsMax[i])
    {
      return false;
    }
  }
  return true;
}

} // namespace Geometry
#endif // GEOMETRY_INTERSECTIONAABB_HPP
