#ifndef GEOMETRY_INTERSECTSEGMENT_HPP
#define GEOMETRY_INTERSECTSEGMENT_HPP

#include <Geometry/Intersect/IntersectInterval.hpp>
#include <Geometry/Intersect/IntersectPlane.hpp>
#include <Geometry/Interval.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

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
intersect(const Segment<T, D>& lhs, const Segment<T, D>& rhs, Segment<T, D>& intersectionSeg, T eps = linal::eps<T>::value) noexcept
{
  linal::vec<T, D> lhsSource = lhs.get_source();
  linal::vec<T, D> lhsTarget = lhs.get_target();
  linal::vec<T, D> rhsSource = rhs.get_source();
  linal::vec<T, D> rhsTarget = rhs.get_target();

  linal::vec<T, D> deltaSource = rhsSource - lhsSource;
  linal::vec<T, D> lhsDir = lhsTarget - lhsSource;
  linal::vec<T, D> rhsDir = rhsTarget - rhsSource;

  if constexpr (D == 3)
  {
    Plane<T> plane{lhsSource, linal::cross(deltaSource, lhsDir)};
    if (intersect(plane, Line<T, D>{rhsSource, rhsDir}))
    {
      return 3;
    }
  }

  T cross = lhsDir[0] * rhsDir[1] - lhsDir[1] * rhsDir[0];
  T sqrCross = cross * cross;
  T sqrLenLhs = lhsDir[0] * lhsDir[0] + lhsDir[1] * lhsDir[1];
  T sqrLenRhs = rhsDir[0] * rhsDir[0] + rhsDir[1] * rhsDir[1];

  if (!linal::isZero(sqrCross, eps * sqrLenLhs * sqrLenRhs))
  {
    // Lines are not parallel
    T s = (deltaSource[0] * rhsDir[1] - deltaSource[1] * rhsDir[0]) / cross;
    if (linal::isLess(s, T(0), eps) || linal::isGreater(s, T(1), eps))
    {
      return 0;
    }

    T t = (deltaSource[0] * lhsDir[1] - deltaSource[1] * lhsDir[0]) / cross;
    if (linal::isLess(t, T(0), eps) || linal::isGreater(t, T(1), eps))
    {
      return 0;
    }

    intersectionSeg.set_source(linal::vec<T, D>(lhsSource + s * lhsDir));
    {
      return 1;
    }
  }

  cross = deltaSource[0] * lhsDir[1] - deltaSource[1] * lhsDir[0];
  sqrCross = cross * cross;
  if (!linal::isZero(sqrCross, eps * sqrLenLhs * sqrLenRhs))
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
  uint32_t res = intersect(sInterval, Interval{T(0), T(1)}, iInterval);
  if (res == 1 || res == 2)
  {
    intersectionSeg.set_source(linal::vec<T, D>(lhsSource + iInterval.get_start() * lhsDir));
  }
  if (res == 2)
  {
    intersectionSeg.set_target(linal::vec<T, D>(lhsSource + iInterval.get_end() * lhsDir));
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
intersect(const Segment<T, D>& seg, const Line<T, D>& line, Segment<T, D>& result, T eps = linal::eps<T>::value) noexcept
{
  linal::vec<T, D> segSource = seg.get_source();
  linal::vec<T, D> segTarget = seg.get_target();
  linal::vec<T, D> lineOrigin = line.get_origin();
  linal::vec<T, D> lineDir = line.get_direction();

  linal::vec<T, D> deltaSource = lineOrigin - segSource;
  linal::vec<T, D> segDir = segTarget - segSource;

  if constexpr (D == 3)
  {
    Plane<T> plane{segSource, linal::cross(deltaSource, segDir)};
    if (intersect(plane, line))
      return 3;
  }

  T cross = segDir[0] * lineDir[1] - segDir[1] * lineDir[0];
  T sqrCross = cross * cross;
  T sqrLenSeg = segDir[0] * segDir[0] + segDir[1] * segDir[1];

  if (!linal::isZero(sqrCross, eps * sqrLenSeg))
  {
    // Lines are not parallel
    T s = (deltaSource[0] * lineDir[1] - deltaSource[1] * lineDir[0]) / cross;
    if (linal::isLess(s, T(0), eps) || linal::isGreater(s, T(1), eps))
      return 0;

    result.set_source(linal::vec<T, D>(segSource + s * segDir));
    return 1;
  }

  cross = deltaSource[0] * segDir[1] - deltaSource[1] * segDir[0];
  sqrCross = cross * cross;
  if (!linal::isZero(sqrCross, eps * sqrLenSeg))
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

#endif // GEOMETRY_INTERSECTSEGMENT_HPP
