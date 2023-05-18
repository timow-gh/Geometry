#ifndef GEOMETRY_FWDRAY_HPP
#define GEOMETRY_FWDRAY_HPP

#include <cmath>
#include <cstddef>

namespace Geometry
{
template <typename T, std::size_t D>
class Ray;
template <typename T>
using Ray3 = class Ray<T, 3>;

template <typename T>
using Ray2 = Ray<T, 2>;
using Ray2f = Ray2<float>;
using Ray2d = Ray2<double>;

template <typename T>
using Ray3 = Ray<T, 3>;
using Ray3f = Ray3<float>;
using Ray3d = Ray3<double>;
} // namespace Geometry

#endif // GEOMETRY_FWDRAY_HPP
