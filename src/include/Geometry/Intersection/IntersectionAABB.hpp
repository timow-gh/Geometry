#ifndef GEOMETRY_INTERSECTIONAABB_HPP
#define GEOMETRY_INTERSECTIONAABB_HPP

#include <Geometry/AABB.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
bool isIntersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs)
{
  LinAl::Vec<TFloat, D> lhsOrigin = lhs.getOrigin();
  LinAl::Vec<TFloat, D> rhsOrigin = rhs.getOrigin();

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
