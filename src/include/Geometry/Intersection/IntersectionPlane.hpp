#ifndef GEOMETRY_INTERSECTIONPLANE_HPP
#define GEOMETRY_INTERSECTIONPLANE_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <optional>

namespace Geometry
{
template <typename T>
std::optional<T> calcIntersectionParameter(LinAl::Vec<T, 3> planeOrigin,
                                           LinAl::Vec<T, 3> planeNormal,
                                           LinAl::Vec3<T> lineOrigin,
                                           LinAl::Vec3<T> lineDir,
                                           T eps)
{
    // Check parallel
    if (Core::isZero(LinAl::dot(lineDir, planeNormal), eps))
        return std::nullopt;

    const LinAl::Vec3<T> vec = planeOrigin - lineOrigin;
    return (LinAl::dot(vec, planeNormal)) / LinAl::dot(lineDir, planeNormal);
}

template <typename T>
CORE_CONSTEXPR std::optional<LinAl::Vec3<T>> intersection(Plane<T> plane, Line3<T> line, T eps = Core::eps_traits<T>::value())
{
    LinAl::Vec<T, 3> planeOrigin = plane.getOrigin();
    LinAl::Vec<T, 3> planeNormal = plane.getNormal();
    LinAl::Vec<T, 3> lineOrigin = line.getOrigin();
    LinAl::Vec<T, 3> lineDir = line.getDirection();

    // Check parallel
    if (Core::isZero(LinAl::dot(lineDir, planeNormal), eps))
        return std::nullopt;

    const auto paramD = LinAl::dot(LinAl::Vec3<T>{planeOrigin - lineOrigin}, planeNormal) / LinAl::dot(lineDir, planeNormal);
    return lineOrigin + paramD * lineDir;
}

template <typename T>
CORE_CONSTEXPR std::optional<LinAl::Vec3<T>> intersection(Plane<T> plane, Ray3<T> ray, T eps = Core::eps_traits<T>::value())
{
    LinAl::Vec<T, 3> planeOrigin = plane.getOrigin();
    LinAl::Vec<T, 3> planeNormal = plane.getNormal();
    LinAl::Vec<T, 3> rayOrigin = ray.getOrigin();
    LinAl::Vec<T, 3> rayDir = ray.getDirection();

    if (auto paramD = calcIntersectionParameter(planeOrigin, planeNormal, rayOrigin, rayDir, eps))
    {
        if (Core::isGreater(*paramD, T(0), eps))
            return rayOrigin + paramD.value() * rayDir;
    }
    return std::nullopt;
}

template <typename T>
CORE_CONSTEXPR std::optional<LinAl::Vec3<T>> intersection(Plane<T> plane, Segment3<T> seg, T eps = Core::eps_traits<T>::value())
{
    LinAl::Vec<T, 3> planeOrigin = plane.getOrigin();
    LinAl::Vec<T, 3> planeNormal = plane.getNormal();
    LinAl::Vec<T, 3> segSource = seg.getSource();
    LinAl::Vec<T, 3> segDir = seg.getTarget() - segSource;

    if (auto paramD = calcIntersectionParameter(planeOrigin, planeNormal, segSource, segDir, eps))
    {
        if (Core::isGreater(*paramD, T(0), eps) && Core::isLessEq(*paramD, T(1), eps))
            return segSource + paramD.value() * segDir;
    }
    return std::nullopt;
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONPLANE_HPP
