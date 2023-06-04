#ifndef GEOMETRY_INTERSECTINTERVAL_HPP
#define GEOMETRY_INTERSECTINTERVAL_HPP

#include <Geometry/Interval.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/utils/eps.hpp>

namespace Geometry
{

//! return value:
//! 0 -> no intersection
//! 1 -> intersection at single point (start of intersection)
//! 2 -> intersection is an interval
template <typename T>
GEO_NODISCARD constexpr std::uint32_t intersect(Interval<T> lhs, Interval<T> rhs, Interval<T>& intersection) noexcept
{
  const T a = lhs.get_start();
  const T b = lhs.get_end();
  const T c = rhs.get_start();
  const T d = rhs.get_end();

  if (linal::isLess(b, c) || linal::isGreater(a, d))
  {
    return 0;
  }

  if (linal::isGreater(b, c))
  {
    if (linal::isLess(a, d))
    {
      if (linal::isLess(a, c))
      {
        intersection.set_start(c);
      }
      else
      {
        intersection.set_start(a);
      }
      if (linal::isGreater(b, d))
      {
        intersection.set_end(d);
      }
      else
      {
        intersection.set_end(b);
      }
      return 2;
    }
    else
    {
      // a == d
      intersection.set_start(a);
      return 1;
    }
  }
  else
  {
    // b = c
    intersection.set_start(b);
    return 1;
  }
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTINTERVAL_HPP
