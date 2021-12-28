#ifndef FILAPP_CIRCLEPOINTS_HPP
#define FILAPP_CIRCLEPOINTS_HPP

#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
//! Points on a circle with the origin at (0,0,0)
template <typename T>
void calcCirclePoints(LinAl::Vec3Vector<T>& points,
                      T circleRadius,
                      std::size_t azimuthCount)
{
    T azimuthStep =
        T(2.0) * T(Core::PI<T>) / static_cast<double_t>(azimuthCount);
    for (std::size_t i{0}; i < azimuthCount; ++i)
    {
        T azimuthAngle = i * azimuthStep;
        points.push_back(LinAl::Vec3<T>{std::cos(azimuthAngle) * circleRadius,
                                        std::sin(azimuthAngle) * circleRadius,
                                        0.0});
    }
}
} // namespace Geometry

#endif // FILAPP_CIRCLEPOINTS_HPP
