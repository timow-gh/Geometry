#ifndef GEOMETRY_DISTANCESPHERE_H
#define GEOMETRY_DISTANCESPHERE_H

#include <Geometry/Sphere.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T>
[[nodiscard]] T signedDistance(const LinAl::Vec3<T>& vec,
                               const Sphere<T>& sphere)
{
    return LinAl::norm2(LinAl::Vec3<T>{vec - sphere.getOrigin()}) -
           sphere.getRadius();
}
template <typename T>
[[nodiscard]] T distance(const LinAl::Vec3<T>& vec, const Sphere<T>& sphere)
{
    return std::abs(signedDistance(vec, sphere));
}
} // namespace Geometry

#endif // GEOMETRY_DISTANCESPHERE_H
