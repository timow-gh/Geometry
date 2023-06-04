#ifndef GEOMETRY_CIRCLEPOINTS_HPP
#define GEOMETRY_CIRCLEPOINTS_HPP

#include <linal/containers.hpp>
#include <linal/vec3.hpp>
#include <linal/utils/constants.hpp>

namespace Geometry
{

//! Points on a circle with the origin at (0,0,0)
template <typename TFloat>
void discretize_circle(linal::vec3vector<TFloat>& points, TFloat circleRadius, std::size_t azimuthCount)
{
  TFloat azimuthStep = TFloat(2.0) * TFloat(linal::PI<TFloat>) / static_cast<double>(azimuthCount);
  for (std::size_t i{0}; i < azimuthCount; ++i)
  {
    TFloat azimuthAngle = i * azimuthStep;
    points.push_back(linal::vec3<TFloat>{std::cos(azimuthAngle) * circleRadius, std::sin(azimuthAngle) * circleRadius, 0.0});
  }
}

} // namespace Geometry

#endif // GEOMETRY_CIRCLEPOINTS_HPP
