#ifndef GEOMETRY_INTERSECTIONLINE_HPP
#define GEOMETRY_INTERSECTIONLINE_HPP

#include <Core/Math/Eps.hpp>
#include <Geometry/Intersection/IntersectionPlane.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>
#include <optional>

namespace Geometry
{
//! Line-Line intersection (Schneider - Geometric Tools for Computer Graphics)
//! return values:
//! 0 -> No intersection, lines parallel and different
//! 1 -> Lines have an intersection
//! 2 -> Lines are the same
//! 3 -> No intersection, skew lines
template <typename T, std::size_t D>
GEO_NODISCARD uint32_t intersection(Line<T, D> lhs, Line<T, D> rhs, linal::Vec<T, D>& intersectionVec, T eps = Core::eps_traits<T>::value())
{
  linal::Vec<T, D> rhsOrigin = rhs.getOrigin();
  linal::Vec<T, D> lhsOrigin = lhs.getOrigin();
  linal::Vec<T, D> rhsDir = rhs.getDirection();
  linal::Vec<T, D> lhsDir = lhs.getDirection();

  linal::Vec<T, D> deltaOrigin = rhsOrigin - lhsOrigin;

  if constexpr (D == 3)
  {
    Plane<T> plane{lhsOrigin, linal::cross(deltaOrigin, lhsDir)};
    if (intersection(plane, rhs))
      return 3;
  }

  T cross = lhsDir[0] * rhsDir[1] - lhsDir[1] * rhsDir[0];
  T sqrCross = cross * cross;
  if (!Core::isZero(sqrCross, eps))
  {
    // Lines are intersecting
    T s = (deltaOrigin[0] * rhsDir[1] - deltaOrigin[1] * rhsDir[0]) / cross;
    intersectionVec = linal::Vec<T, D>(lhsOrigin + s * lhsDir);
    return 1;
  }

  // FixMe T sqrDeltaOrigin = linal::norm2Squared(deltaOrigin);
  cross = deltaOrigin[0] * lhsDir[1] - deltaOrigin[1] * lhsDir[0];
  sqrCross = cross * cross;
  if (!Core::isZero(sqrCross, eps))
  {
    // Lines are parallel in the plane
    // Lines are different
    return 0;
  }
  // Lines are the same
  return 2;
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONLINE_HPP
