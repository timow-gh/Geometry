#ifndef GEOMETRY_INTERSECTLINE_HPP
#define GEOMETRY_INTERSECTLINE_HPP

#include "Geometry/Intersect/IntersectPlane.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
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
GEO_NODISCARD uint32_t intersect(Line<T, D> lhs, Line<T, D> rhs, linal::vec<T, D>& intersectionVec, T eps = linal::eps<T>::value) noexcept
{
  linal::vec<T, D> rhsOrigin = rhs.get_origin();
  linal::vec<T, D> lhsOrigin = lhs.get_origin();
  linal::vec<T, D> rhsDir = rhs.get_direction();
  linal::vec<T, D> lhsDir = lhs.get_direction();

  linal::vec<T, D> deltaOrigin = rhsOrigin - lhsOrigin;

  if constexpr (D == 3)
  {
    Plane<T> plane{lhsOrigin, linal::cross(deltaOrigin, lhsDir)};
    if (intersect(plane, rhs))
    {
      return 3;
    }
  }

  T cross = lhsDir[0] * rhsDir[1] - lhsDir[1] * rhsDir[0];
  T sqrCross = cross * cross;
  if (!linal::isZero(sqrCross, eps))
  {
    // Lines are intersecting
    T s = (deltaOrigin[0] * rhsDir[1] - deltaOrigin[1] * rhsDir[0]) / cross;
    intersectionVec = linal::vec<T, D>(lhsOrigin + s * lhsDir);
    return 1;
  }

  // FixMe T sqrDeltaOrigin = linal::norm2Squared(deltaOrigin);
  cross = deltaOrigin[0] * lhsDir[1] - deltaOrigin[1] * lhsDir[0];
  sqrCross = cross * cross;
  if (!linal::isZero(sqrCross, eps))
  {
    // Lines are parallel in the plane
    // Lines are different
    return 0;
  }
  // Lines are the same
  return 2;
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTLINE_HPP
