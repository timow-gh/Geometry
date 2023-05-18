#ifndef GEOMETRY_INTERSECTIONAABB_HPP
#define GEOMETRY_INTERSECTIONAABB_HPP

#include <Geometry/AABB.hpp>
#include <linal/vec.hpp>
#include <linal/utils/eps.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
bool isIntersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs)
{
  linal::vec<TFloat, D> lhsOrigin = lhs.getOrigin();
  linal::vec<TFloat, D> rhsOrigin = rhs.getOrigin();

  std::array<TFloat, D> lhsExtends = lhs.getExtends();
  std::array<TFloat, D> rhsExtends = rhs.getExtends();

  for (std::size_t i = 0; i < D; ++i)
  {
    if (linal::isGreater(std::abs(lhsOrigin[i] - rhsOrigin[i]), lhsExtends[i] + rhsExtends[i]))
      return false;
  }
  return true;
}

} // namespace Geometry
#endif // GEOMETRY_INTERSECTIONAABB_HPP
