#ifndef GEOMETRY_INTERSECTIONAABB_HPP
#define GEOMETRY_INTERSECTIONAABB_HPP

#include <Core/Math/Eps.hpp>
#include <Geometry/AABB.hpp>
#include <linal/Vec.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
bool isIntersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs)
{
  linal::Vec<TFloat, D> lhsOrigin = lhs.getOrigin();
  linal::Vec<TFloat, D> rhsOrigin = rhs.getOrigin();

  Core::TArray<TFloat, D> lhsExtends = lhs.getExtends();
  Core::TArray<TFloat, D> rhsExtends = rhs.getExtends();

  for (std::size_t i = 0; i < D; ++i)
  {
    if (Core::isGreater(std::abs(lhsOrigin[i] - rhsOrigin[i]), lhsExtends[i] + rhsExtends[i]))
      return false;
  }
  return true;
}

} // namespace Geometry
#endif // GEOMETRY_INTERSECTIONAABB_HPP
