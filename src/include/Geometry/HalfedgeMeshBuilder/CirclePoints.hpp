#ifndef FILAPP_CIRCLEPOINTS_HPP
#define FILAPP_CIRCLEPOINTS_HPP

#include <Core/Math/Constants.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
//! Points on a circle with the origin at (0,0,0)
template <typename TFloat>
void calcCirclePoints(LinAl::Vec3Vector<TFloat>& points, TFloat circleRadius, std::size_t azimuthCount)
{
  TFloat azimuthStep = TFloat(2.0) * TFloat(Core::PI<TFloat>) / static_cast<double>(azimuthCount);
  for (std::size_t i{0}; i < azimuthCount; ++i)
  {
    TFloat azimuthAngle = i * azimuthStep;
    points.push_back(LinAl::Vec3<TFloat>{std::cos(azimuthAngle) * circleRadius, std::sin(azimuthAngle) * circleRadius, 0.0});
  }
}
} // namespace Geometry

#endif // FILAPP_CIRCLEPOINTS_HPP
