#ifndef GEOMETRY_INTERSECTIONPLANE_HPP
#define GEOMETRY_INTERSECTIONPLANE_HPP

#include "Core/Math/Eps.hpp"
#include "Geometry/FwdGeometry.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <optional>

namespace Geometry
{
template <typename T>
std::optional<T>
calcIntersectionParameter(const LinAl::Vec<T, 3>& planeOrigin,
                          const LinAl::Vec<T, 3>& planeNormal,
                          const LinAl::Vec3<T>& lineOrigin,
                          const LinAl::Vec3<T>& lineDir,
                          T eps = Core::eps_traits<T>::value());
template <typename T>
constexpr std::optional<LinAl::Vec3<T>>
calcIntersection(const Plane<T>& plane,
                 const Line3<T>& line,
                 T eps = Core::eps_traits<T>::value())
{
    const LinAl::Vec<T, 3>& planeOrigin = plane.getOrigin();
    const LinAl::Vec<T, 3>& planeNormal = plane.getNormal();
    const LinAl::Vec<T, 3>& lineOrigin = line.getOrigin();
    const LinAl::Vec<T, 3>& lineDir = line.getDirection();

    if (auto paramD = calcIntersectionParameter(planeOrigin,
                                                planeNormal,
                                                lineOrigin,
                                                lineDir))
        return lineOrigin + paramD.value() * lineDir;
    return std::nullopt;
}
template <typename T>
constexpr std::optional<LinAl::Vec3<T>>
calcIntersection(const Plane<T>& plane,
                 const Ray3<T>& ray,
                 T eps = Core::eps_traits<T>::value())
{
    const LinAl::Vec<T, 3>& planeOrigin = plane.getOrigin();
    const LinAl::Vec<T, 3>& planeNormal = plane.getNormal();
    const LinAl::Vec<T, 3>& rayOrigin = ray.getOrigin();
    const LinAl::Vec<T, 3>& rayDir = ray.getDirection();

    if (auto paramD = calcIntersectionParameter(planeOrigin,
                                                planeNormal,
                                                rayOrigin,
                                                rayDir))
    {
        if (Core::isGreater(*paramD, T(0), eps))
            return rayOrigin + paramD.value() * rayDir;
    }
    return std::nullopt;
}
template <typename T>
constexpr std::optional<LinAl::Vec3<T>>
calcIntersection(const Plane<T>& plane,
                 const Segment3<T>& seg,
                 T eps = Core::eps_traits<T>::value())
{
    const LinAl::Vec<T, 3>& planeOrigin = plane.getOrigin();
    const LinAl::Vec<T, 3>& planeNormal = plane.getNormal();
    const LinAl::Vec<T, 3>& segSource = seg.getSource();
    const LinAl::Vec<T, 3>& segDir = seg.getTarget() - segSource;

    if (auto paramD = calcIntersectionParameter(planeOrigin,
                                                planeNormal,
                                                segSource,
                                                segDir))
    {
        if (Core::isGreater(*paramD, T(0), eps) &&
            Core::isLessEq(*paramD, T(1), eps))
            return segSource + paramD.value() * segDir;
    }
    return std::nullopt;
}
template <typename T>
std::optional<T> calcIntersectionParameter(const LinAl::Vec<T, 3>& planeOrigin,
                                           const LinAl::Vec<T, 3>& planeNormal,
                                           const LinAl::Vec3<T>& lineOrigin,
                                           const LinAl::Vec3<T>& lineDir,
                                           T eps)
{

    // Check parallel
    if (Core::isZero(LinAl::dot(lineDir, planeNormal), eps))
        return std::nullopt;

    const LinAl::Vec3<T> vec = planeOrigin - lineOrigin;
    return (LinAl::dot(vec, planeNormal)) / LinAl::dot(lineDir, planeNormal);
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONPLANE_HPP
