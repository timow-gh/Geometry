#ifndef GEOMETRY_CIRCLEPOINTS_HPP
#define GEOMETRY_CIRCLEPOINTS_HPP

#include <linal/utils/constants.hpp>
#include <linal/vec.hpp>
#include <vector>

namespace Geometry
{

/**
 * @brief Discretizes a circle with the given radius and azimuth count in the xy-plane, using the counter-clockwise direction.
 *
 * @tparam TFloat The type of the floating point number.
 * @param points The vector of points to store the discretized circle.
 * @param circleRadius The radius of the circle.
 * @param azimuthCount The number of azimuths.
 */
template <typename TFloat>
void discretize_circle(std::vector<linal::vec3<TFloat>>& points, TFloat circleRadius, std::size_t azimuthCount)
{
  TFloat azimuthStep = TFloat(2.0) * TFloat(linal::PI<TFloat>) / static_cast<TFloat>(azimuthCount);
  for (std::size_t i{0}; i < azimuthCount; ++i)
  {
    TFloat azimuthAngle = static_cast<TFloat>(i) * azimuthStep;
    points.push_back(linal::vec3<TFloat>{std::cos(azimuthAngle) * circleRadius, std::sin(azimuthAngle) * circleRadius, TFloat{0.0}});
  }
}

enum class CircleDiscretizationDirection
{
  CCW,
  CW
};

/**
 * @brief Discretizes a circle with the given radius and azimuth count in the xy-plane, using the counter-clockwise direction.
 *
 * @attention Requires the output iterator to be able to store the number of points given by the azimuth count + 1.
 */
template <typename TFloat, typename TIndex, typename TVecOutIter>
void discretize_circle(linal::vec3<TFloat> center,
                       TFloat circleRadius,
                       TIndex azimuthCount,
                       CircleDiscretizationDirection discrDir,
                       TVecOutIter vecOutStart)
{
  TFloat azimuthStep = TFloat{2.0} * TFloat(linal::PI<TFloat>) / static_cast<TFloat>(azimuthCount);
  if (discrDir == CircleDiscretizationDirection::CW)
  {
    azimuthStep = -azimuthStep;
  }
  for (TIndex i{0}; i < azimuthCount; ++i)
  {
    TFloat azimuthAngle = static_cast<TFloat>(i) * azimuthStep;
    *vecOutStart = center + linal::vec3<TFloat>{std::cos(azimuthAngle) * circleRadius, std::sin(azimuthAngle) * circleRadius, TFloat{0.0}};
    ++vecOutStart;
  }
}

/**
 * @brief Calculates the points and indices of the triangles of a circle.
 *
 * The triangles are calculated in the xy-plane.
 * The first point written to the TVecOutIter is the center of the circle,
 * the following points are ordered counter-clockwise.
 */
template <typename TFloat, typename TIndex, typename TVecOutIter, typename TIndexOutIter>
void calc_circle_triangles(linal::vec3<TFloat> center,
                           TFloat circleRadius,
                           TIndex azimuthCount,
                           CircleDiscretizationDirection discrDir,
                           TVecOutIter vecOutIter,
                           TIndex midPointIdx,
                           TIndexOutIter indexOutIter)
{
  *vecOutIter = center;
  ++vecOutIter;

  discretize_circle(center, circleRadius, azimuthCount, discrDir, vecOutIter);

  TIndex startIdx{midPointIdx + 1};
  TIndex endIdx{startIdx + azimuthCount - 1};

  for (TIndex i{startIdx}; i <= endIdx; ++i)
  {
    *indexOutIter++ = midPointIdx;
    *indexOutIter++ = i;
    *indexOutIter++ = i == endIdx ? startIdx : i + 1;
  }
}

} // namespace Geometry

#endif // GEOMETRY_CIRCLEPOINTS_HPP
