#ifndef GEOMETRY_FWDGEOMETRY_HPP
#define GEOMETRY_FWDGEOMETRY_HPP

#include <cstddef>

namespace Geometry
{
template <typename T, std::size_t D>
class Line;
template <typename T>
using Line3 = Line<T, 3>;

template <typename T, std::size_t D>
class Ray;
template <typename T>
using Ray3 = class Ray<T, 3>;

template <typename T, std::size_t D>
class Segment;
template <typename T>
using Segment3 = class Segment<T, 3>;

template <typename T>
class Plane;

template <typename T>
class Sphere;
} // namespace Geometry

#endif // GEOMETRY_FWDGEOMETRY_HPP
