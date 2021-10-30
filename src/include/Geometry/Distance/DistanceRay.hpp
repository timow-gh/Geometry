#ifndef GEOMETRY_DISTANCERAY_HPP
#define GEOMETRY_DISTANCERAY_HPP

#include "Core/Math/Eps.hpp"
#include "Geometry/FwdGeometry.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{
template <typename T, std::size_t D>
[[nodiscard]] constexpr T distance(const Ray<T, D>& ray,
                                   const LinAl::Vec<T, D>& vec)
{
    LinAl::Vec<T, D> vecO{vec - ray.getOrigin()};
    const LinAl::Vec<T, D> rayDirection = ray.getDirection();

    const T dotProduct = LinAl::dot(vecO, rayDirection);
    LinAl::Vec<T, D> distanceVec;
    if (Core::isGreater(dotProduct, T(0)))
        distanceVec = LinAl::rejection(vecO, rayDirection);
    else
        distanceVec = ray.getOrigin() - vec;

    return LinAl::norm2(distanceVec);
}

template <typename T, std::size_t D>
[[nodiscard]] constexpr T distance(const LinAl::Vec<T, D>& vec,
                                   const Ray<T, D>& ray)
{
    return distance(ray, vec);
}

template <typename T, std::size_t D>
[[nodiscard]] constexpr T distance(const Ray<T, D>& ray, const Line<T, D>& line)
{
    // TODO parallel
    const auto& rayOrigin = ray.getOrigin();
    const auto& lineOrigin = line.getOrigin();
    LinAl::Vec<T, D> originVec{rayOrigin - lineOrigin};

    const auto& lineDir = line.getDirection();
    const auto& rayDir = ray.getDirection();

    T rayOriginVec = LinAl::dot(rayDir, originVec);
    T rayLine = -LinAl::dot(rayDir, lineDir);
//    T rayRay = LinAl::dot(rayDir, rayDir);
//    T rayParam = (-rayOriginVec + rayLine) / rayRay;
    T rayParam = (rayOriginVec + rayLine);
    rayParam = std::max(0.0, rayParam);
    LinAl::Vec<T, D> rayPoint = rayOrigin + rayParam * rayDir;

    T lineOriginVec = LinAl::dot(lineDir, originVec);
//    T lineLine = LinAl::dot(lineDir, lineDir);
//    T lineParam = lineOriginVec / lineLine;
    LinAl::Vec<T, D> linePoint = lineOrigin + lineOriginVec * lineDir;

    return LinAl::norm2(LinAl::Vec<T, D>{rayPoint - linePoint});
}

template <typename T, std::size_t D>
[[nodiscard]] constexpr T distance(const Line<T, D>& line, const Ray<T, D>& ray)
{
    return distance(ray, line);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCERAY_HPP
