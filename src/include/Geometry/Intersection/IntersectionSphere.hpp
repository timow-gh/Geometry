#ifndef GEOMETRY_INTERSECTIONSPHERE_HPP
#define GEOMETRY_INTERSECTIONSPHERE_HPP

#include "Core/Math/Eps.hpp"
#include "Geometry/FwdGeometry.hpp"
#include "Geometry/Sphere.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <optional>

namespace Geometry
{

template <typename T>
struct SphereIntersection
{
    std::optional<LinAl::Vec3<T>> first;
    std::optional<LinAl::Vec3<T>> second;

    SphereIntersection() = default;
    explicit SphereIntersection(const LinAl::Vec3<T>& first) : first(first) {}
    SphereIntersection(const LinAl::Vec3<T>& first,
                       const LinAl::Vec3<T>& second)
        : first(first), second(second)
    {
    }
};

template <typename T>
std::optional<SphereIntersection<T>>
calcIntersection(const Sphere<T>& sphere,
                 const Line3<T>& line,
                 T eps = Core::eps_traits<T>::value())
{
    // See Schneider - Geometric Tools, Linear Components and Spheres
    const LinAl::Vec3<T>& sphereOrigin = sphere.getOrigin();
    const T radius = sphere.getRadius();
    const LinAl::Vec3<T>& lineOrigin = line.getOrigin();
    const LinAl::Vec3<T>& lineDir = line.getDirection();

    const LinAl::Vec3<T> deltaOrigin = (lineOrigin - sphereOrigin);
    const T a = LinAl::dot(lineDir, lineDir);
    const T b = 2 * LinAl::dot(lineDir, deltaOrigin);
    const T c = LinAl::dot(deltaOrigin, deltaOrigin) - radius * radius;
    const T discriminant = b * b - 4 * a * c;

    if (Core::isGreater(discriminant, T(0), eps))
    {
        T sqrtDiscr = std::sqrt(discriminant);
        T twoA = 2 * a;
        T t1 = (-b + sqrtDiscr) / twoA;
        T t2 = (-b - sqrtDiscr) / twoA;
        return SphereIntersection<T>(lineOrigin + t1 * lineDir,
                                     lineOrigin + t2 * lineDir);
    }
    else if (Core::isZero(discriminant, eps))
    {
        T t = -b / 2 * a;
        return SphereIntersection<T>(lineOrigin + t * lineDir);
    }
    return std::nullopt;
}

template <typename T>
std::optional<SphereIntersection<T>>
calcIntersection(const Sphere<T>& sphere,
                 const Ray3<T>& ray,
                 T eps = Core::eps_traits<T>::value())
{
    // See Schneider - Geometric Tools, Linear Components and Spheres
    const LinAl::Vec3<T>& sphereOrigin = sphere.getOrigin();
    const T radius = sphere.getRadius();
    const LinAl::Vec3<T>& rayOrigin = ray.getOrigin();
    const LinAl::Vec3<T>& rayDir = ray.getDirection();

    const LinAl::Vec3<T> deltaOrigin = (rayOrigin - sphereOrigin);
    const T a = LinAl::dot(rayDir, rayDir);
    const T b = 2 * LinAl::dot(rayDir, deltaOrigin);
    const T c = LinAl::dot(deltaOrigin, deltaOrigin) - radius * radius;
    const T discriminant = b * b - 4 * a * c;

    if (Core::isGreater(discriminant, T(0), eps))
    {
        T sqrtDiscr = std::sqrt(discriminant);
        T twoA = 2 * a;
        T t1 = (-b + sqrtDiscr) / twoA;
        T t2 = (-b - sqrtDiscr) / twoA;

        SphereIntersection<T> result;
        if (Core::isGreaterEq(t1, T(0), eps))
            result.first = rayOrigin + t1 * rayDir;
        if (Core::isGreaterEq(t2, T(0), eps))
            result.second = rayOrigin + t2 * rayDir;
        return result;
    }
    else if (Core::isZero(discriminant, eps))
    {
        T t = -b / 2 * a;
        if (Core::isGreaterEq(t, T(0), eps))
            return SphereIntersection<T>(rayOrigin + t * rayDir);
    }
    return std::nullopt;
}

template <typename T>
std::optional<SphereIntersection<T>>
calcIntersection(const Sphere<T>& sphere,
                 const Segment3<T>& segment,
                 T eps = Core::eps_traits<T>::value())
{
    // See Schneider - Geometric Tools, Linear Components and Spheres
    const LinAl::Vec3<T>& sphereOrigin = sphere.getOrigin();
    const T radius = sphere.getRadius();
    const LinAl::Vec3<T>& segSource = segment.getSource();
    const LinAl::Vec3<T>& segTarget = segment.getTarget();

    const LinAl::Vec3<T> deltaOrigin = (segSource - sphereOrigin);
    const LinAl::Vec3<T> segDir = (segTarget - segSource);

    const T a = LinAl::dot(segDir, segDir);
    const T b = 2 * LinAl::dot(segDir, deltaOrigin);
    const T c = LinAl::dot(deltaOrigin, deltaOrigin) - radius * radius;
    const T discriminant = b * b - 4 * a * c;

    if (Core::isGreater(discriminant, T(0), eps))
    {
        T sqrtDiscr = std::sqrt(discriminant);
        T twoA = 2 * a;
        T t1 = (-b + sqrtDiscr) / twoA;
        T t2 = (-b - sqrtDiscr) / twoA;

        SphereIntersection<T> result;
        if (Core::isGreaterEq(t1, T(0), eps) && Core::isLessEq(t1, T(1), eps))
            result.first = segSource + t1 * segDir;
        if (Core::isGreaterEq(t2, T(0), eps) && Core::isLessEq(t2, T(1), eps))
            result.second = segSource + t2 * segDir;
        if (!result.first && !result.second)
            return std::nullopt;
        return result;
    }
    else if (Core::isZero(discriminant, eps))
    {
        T t = -b / 2 * a;
        if (Core::isGreaterEq(t, T(0), eps) && Core::isLessEq(t, T(1), eps))
            return SphereIntersection<T>(segSource + t * segDir);
    }
    return std::nullopt;
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONSPHERE_HPP
