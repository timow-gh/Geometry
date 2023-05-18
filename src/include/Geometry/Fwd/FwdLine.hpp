#ifndef GEOMETRY_FWDLINE_H
#define GEOMETRY_FWDLINE_H

#include <cmath>
#include <cstddef>

namespace Geometry
{
template <typename T, std::size_t D>
class Line;
template <typename T>
using Line3 = Line<T, 3>;

template <typename T>
using Line2 = Line<T, 2>;
using Line2f = Line2<float>;
using Line2d = Line2<double>;

template <typename T>
using Line3 = Line<T, 3>;
using Line3f = Line3<float>;
using Line3d = Line3<double>;
} // namespace Geometry

#endif // GEOMETRY_FWDLINE_H
