#ifndef GEOMETRY_INTERSECTIONINTERVAL_HPP
#define GEOMETRY_INTERSECTIONINTERVAL_HPP

#include <Geometry/Interval.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/utils/Eps.hpp>

namespace Geometry
{
//! return value:
//! 0 -> no intersection
//! 1 -> intersection at single point (start of intersection)
//! 2 -> intersection is an interval
template <typename T>
std::uint32_t intersection(Interval<T> lhs, Interval<T> rhs, Interval<T>& intersection)
{
  const T a = lhs.getStart();
  const T b = lhs.getEnd();
  const T c = rhs.getStart();
  const T d = rhs.getEnd();

  if (linal::isLess(b, c) || linal::isGreater(a, d))
    return 0;

  if (linal::isGreater(b, c))
  {
    if (linal::isLess(a, d))
    {
      if (linal::isLess(a, c))
        intersection.setStart(c);
      else
        intersection.setStart(a);
      if (linal::isGreater(b, d))
        intersection.setEnd(d);
      else
        intersection.setEnd(b);
      return 2;
    }
    else
    {
      // a == d
      intersection.setStart(a);
      return 1;
    }
  }
  else
  {
    // b = c
    intersection.setStart(b);
    return 1;
  }
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONINTERVAL_HPP
