#ifndef GEOMETRY_DISTANCEPLANE_HPP
#define GEOMETRY_DISTANCEPLANE_HPP

#include "Geometry/FwdGeometry.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{
template <typename T, std::size_t D>
T distance(const Plane<T>& plane, const LinAl::Vec<T, D>& vec)
{
    LinAl::Vec<T, D> vecToPlane = plane.getOrigin() - vec;
    return std::abs(LinAl::dot(vecToPlane, plane.getNormal()));
}

template <typename T, std::size_t D>
T distance(const LinAl::Vec<T, D>& vec, const Plane<T>& plane)
{
    return distance(plane, vec);
}
} // namespace Geometry

#endif // GEOMETRY_DISTANCEPLANE_HPP
