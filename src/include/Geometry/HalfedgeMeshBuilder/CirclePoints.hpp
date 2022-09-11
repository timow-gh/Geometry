#ifndef FILAPP_CIRCLEPOINTS_HPP
#define FILAPP_CIRCLEPOINTS_HPP

#include <Core/Math/Constants.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
//! Points on a circle with the origin at (0,0,0)
template <typename TFloatType>
void calcCirclePoints(LinAl::Vec3Vector<TFloatType>& points, TFloatType circleRadius, std::size_t azimuthCount)
{
  TFloatType azimuthStep = TFloatType(2.0) * TFloatType(Core::PI<TFloatType>) / static_cast<double_t>(azimuthCount);
  for (std::size_t i{0}; i < azimuthCount; ++i)
  {
    TFloatType azimuthAngle = i * azimuthStep;
    points.push_back(LinAl::Vec3<TFloatType>{std::cos(azimuthAngle) * circleRadius, std::sin(azimuthAngle) * circleRadius, 0.0});
  }
}
} // namespace Geometry

#endif // FILAPP_CIRCLEPOINTS_HPP
