#ifndef GEOMETRY_INTERSECTIONSEGMENT_HPP
#define GEOMETRY_INTERSECTIONSEGMENT_HPP

#include <Core/Math/Eps.hpp>
#include <Geometry/Intersection/IntersectionInterval.hpp>
#include <Geometry/Intersection/IntersectionPlane.hpp>
#include <Geometry/Interval.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{
//! Line-Line intersection (Schneider - Geometric Tools for Computer Graphics)
//! return value:
//! 0 -> No intersection
//! 1 -> Unique intersection
//! 2 -> Segments overlap, the intersection is a segment
//! 3 -> No intersection, skew segment lines
template <typename T, std::size_t D>
GEO_NODISCARD uint32_t
intersection(const Segment<T, D>& lhs, const Segment<T, D>& rhs, Segment<T, D>& intersectionSeg, T eps = Core::eps_traits<T>::value())
{
  linal::Vec<T, D> lhsSource = lhs.getSource();
  linal::Vec<T, D> lhsTarget = lhs.getTarget();
  linal::Vec<T, D> rhsSource = rhs.getSource();
  linal::Vec<T, D> rhsTarget = rhs.getTarget();

  linal::Vec<T, D> deltaSource = rhsSource - lhsSource;
  linal::Vec<T, D> lhsDir = lhsTarget - lhsSource;
  linal::Vec<T, D> rhsDir = rhsTarget - rhsSource;

  // TODO extract function: is planar to plane (see line intersection)
  if constexpr (D == 3)
  {
    Plane<T> plane{lhsSource, linal::cross(deltaSource, lhsDir)};
    if (intersection(plane, (Line<T, D>{rhsSource, rhsDir})))
      return 3;
  }

  T cross = lhsDir[0] * rhsDir[1] - lhsDir[1] * rhsDir[0];
  T sqrCross = cross * cross;
  T sqrLenLhs = lhsDir[0] * lhsDir[0] + lhsDir[1] * lhsDir[1];
  T sqrLenRhs = rhsDir[0] * rhsDir[0] + rhsDir[1] * rhsDir[1];

  if (!Core::isZero(sqrCross, eps * sqrLenLhs * sqrLenRhs))
  {
    // Lines are not parallel
    T s = (deltaSource[0] * rhsDir[1] - deltaSource[1] * rhsDir[0]) / cross;
    if (Core::isLess(s, T(0), eps) || Core::isGreater(s, T(1), eps))
      return 0;

    T t = (deltaSource[0] * lhsDir[1] - deltaSource[1] * lhsDir[0]) / cross;
    if (Core::isLess(t, T(0), eps) || Core::isGreater(t, T(1), eps))
      return 0;

    intersectionSeg.setSource(linal::Vec<T, D>(lhsSource + s * lhsDir));
    return 1;
  }

  // TODO extract function: is parallel in plane (see line intersection)
  // FixMe T sqrDeltaSource = linal::norm2Squared(deltaSource);
  cross = deltaSource[0] * lhsDir[1] - deltaSource[1] * lhsDir[0];
  sqrCross = cross * cross;
  if (!Core::isZero(sqrCross, eps * sqrLenLhs * sqrLenRhs))
  {
    // Lines are parallel in the plane
    // Lines are different
    return 0;
  }

  T s0 = linal::dot(lhsDir, deltaSource) / sqrLenLhs;
  T s1 = s0 + linal::dot(lhsDir, rhsDir) / sqrLenLhs;
  T sMin = std::min(s0, s1);
  T sMax = std::max(s0, s1);
  Interval sInterval{sMin, sMax};
  Interval<T> iInterval;
  uint32_t res = intersection(sInterval, Interval{T(0), T(1)}, iInterval);
  if (res == 1 || res == 2)
  {
    intersectionSeg.setSource(linal::Vec<T, D>(lhsSource + iInterval.getStart() * lhsDir));
  }
  if (res == 2)
  {
    intersectionSeg.setTarget(linal::Vec<T, D>(lhsSource + iInterval.getEnd() * lhsDir));
  }
  return res;
}

//! return value:
//! 0 -> No intersection
//! 1 -> Unique intersection
//! 2 -> Overlap, the intersection is the segment
//! 3 -> No intersection, skew segment lines
template <typename T, std::size_t D>
GEO_NODISCARD uint32_t
intersection(const Segment<T, D>& seg, const Line<T, D>& line, Segment<T, D>& result, T eps = Core::eps_traits<T>::value())
{
  linal::Vec<T, D> segSource = seg.getSource();
  linal::Vec<T, D> segTarget = seg.getTarget();
  linal::Vec<T, D> lineOrigin = line.getOrigin();
  linal::Vec<T, D> lineDir = line.getDirection();

  linal::Vec<T, D> deltaSource = lineOrigin - segSource;
  linal::Vec<T, D> segDir = segTarget - segSource;

  // TODO extract function: is planar to plane (see line intersection)
  if GEO_CONSTEXPR (D == 3)
  {
    Plane<T> plane{segSource, linal::cross(deltaSource, segDir)};
    if (intersection(plane, line))
      return 3;
  }

  T cross = segDir[0] * lineDir[1] - segDir[1] * lineDir[0];
  T sqrCross = cross * cross;
  T sqrLenSeg = segDir[0] * segDir[0] + segDir[1] * segDir[1];

  if (!Core::isZero(sqrCross, eps * sqrLenSeg))
  {
    // Lines are not parallel
    T s = (deltaSource[0] * lineDir[1] - deltaSource[1] * lineDir[0]) / cross;
    if (Core::isLess(s, T(0), eps) || Core::isGreater(s, T(1), eps))
      return 0;

    result.setSource(linal::Vec<T, D>(segSource + s * segDir));
    return 1;
  }

  // TODO extract function: is parallel in plane (see line intersection)
  // FixMe T sqrDeltaSource = linal::norm2Squared(deltaSource);
  cross = deltaSource[0] * segDir[1] - deltaSource[1] * segDir[0];
  sqrCross = cross * cross;
  if (!Core::isZero(sqrCross, eps * sqrLenSeg))
  {
    // Lines are parallel in the plane
    // Lines are different
    return 0;
  }

  // Line is collinear to the segment
  result = seg;
  return 2;
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONSEGMENT_HPP
