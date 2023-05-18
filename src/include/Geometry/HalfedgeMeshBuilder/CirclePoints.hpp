#ifndef FILAPP_CIRCLEPOINTS_HPP
#define FILAPP_CIRCLEPOINTS_HPP

#include <linal/utils/Constants.hpp>
#include <linal/Containers.hpp>
#include <linal/Vec3.hpp>

namespace Geometry
{
//! Points on a circle with the origin at (0,0,0)
template <typename TFloat>
void calcCirclePoints(linal::Vec3Vector<TFloat>& points, TFloat circleRadius, std::size_t azimuthCount)
{
  TFloat azimuthStep = TFloat(2.0) * TFloat(linal::PI<TFloat>) / static_cast<double>(azimuthCount);
  for (std::size_t i{0}; i < azimuthCount; ++i)
  {
    TFloat azimuthAngle = i * azimuthStep;
    points.push_back(linal::Vec3<TFloat>{std::cos(azimuthAngle) * circleRadius, std::sin(azimuthAngle) * circleRadius, 0.0});
  }
}
} // namespace Geometry

#endif // FILAPP_CIRCLEPOINTS_HPP
