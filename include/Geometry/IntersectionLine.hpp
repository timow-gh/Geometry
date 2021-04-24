#ifndef GEOMETRY_INTERSECTIONLINE_HPP
#define GEOMETRY_INTERSECTIONLINE_HPP

#include "Core/Math/Eps.hpp"

#include "LinAl/LinearAlgebra.hpp"

#include "Fwd.hpp"
#include "IntersectionPlane.hpp"
#include "Plane.hpp"

namespace Geometry {
//! Line-Line intersection (Schneider - Geometric Tools for Computer Graphics)
//! return values:
//! 0 -> No intersection, lines parallel and different
//! 1 -> Lines have an intersection
//! 2 -> Lines are the same
//! 3 -> No intersection, skew lines
template <typename T, std::size_t D>
[[nodiscard]] uint32_t calcIntersection(const Line<T, D> &lhs,
                                        const Line<T, D> &rhs,
                                        LinAl::Vec<T, D> &intersection,
                                        T eps = Core::eps_traits<T>::value()) {
  const LinAl::Vec<T, D> &rhsOrigin = rhs.getOrigin();
  const LinAl::Vec<T, D> &lhsOrigin = lhs.getOrigin();
  const LinAl::Vec<T, D> &rhsDir = rhs.getDirection();
  const LinAl::Vec<T, D> &lhsDir = lhs.getDirection();

  LinAl::Vec<T, D> deltaOrigin = rhsOrigin - lhsOrigin;

  if constexpr (D == 3) {
    Plane<T> plane{lhsOrigin, LinAl::cross(deltaOrigin, lhsDir)};
    if (plane.intersection(rhs))
      return 3;
  }

  T cross = lhsDir[0] * rhsDir[1] - lhsDir[1] * rhsDir[0];
  T sqrCross = cross * cross;
  if (!Core::isZero(sqrCross, eps)) {
    // Lines are intersecting
    T s = (deltaOrigin[0] * rhsDir[1] - deltaOrigin[1] * rhsDir[0]) / cross;
    intersection = LinAl::Vec<T, D>(lhsOrigin + s * lhsDir);
    return 1;
  }

  T sqrDeltaOrigin = LinAl::norm2Squared(deltaOrigin);
  cross = deltaOrigin[0] * lhsDir[1] - deltaOrigin[1] * lhsDir[0];
  sqrCross = cross * cross;
  if (!Core::isZero(sqrCross, eps)) {
    // Lines are parallel in the plane
    // Lines are different
    return 0;
  }
  // Lines are the same
  return 2;
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONLINE_HPP
