#ifndef GEOMETRY_INTERSECTIONINTERVAL_HPP
#define GEOMETRY_INTERSECTIONINTERVAL_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Interval.hpp>

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

    if (Core::isLess(b, c) || Core::isGreater(a, d))
        return 0;

    if (Core::isGreater(b, c))
    {
        if (Core::isLess(a, d))
        {
            if (Core::isLess(a, c))
                intersection.setStart(c);
            else
                intersection.setStart(a);
            if (Core::isGreater(b, d))
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