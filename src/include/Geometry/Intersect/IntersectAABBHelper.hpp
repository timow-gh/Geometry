#ifndef GEOMETRY_INTERSECTAABBHELPER_HPP
#define GEOMETRY_INTERSECTAABBHELPER_HPP

#include "Geometry/AABB.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Utils/Assert.hpp"
#include <limits>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

namespace details
{

template <typename TFloat>
struct AABBRayIntersectionInterval
{
  TFloat tEnter{std::numeric_limits<TFloat>::max()};
  TFloat tExit{std::numeric_limits<TFloat>::lowest()};
};

/** @brief Computes the interval along the ray where it intersects the AABB.
 *
 *  Returns the interval [tEnter, tExit] where the ray intersects the AABB.
 *  If the ray does not intersect the AABB, tEnter > tExit.
 */
template <typename TFloat>
GEO_NODISCARD constexpr AABBRayIntersectionInterval<TFloat> aabb_ray_intersection_interval(const Ray<TFloat, 3>& ray,
                                                                                           const AABB<TFloat, 3>& aabb) noexcept
{
  // Define the dimension D as 3 for 3D space
  constexpr std::uint8_t D = 3;
  using vec_type = linal::vec<TFloat, D>;

  // Get the origin and direction of the ray
  const vec_type rayOrigin = ray.get_origin();
  const vec_type rayDir = ray.get_direction();

  // Compute the inverse of each component of the ray's direction vector
  // This helps avoid division during intersection calculations
  vec_type invDir;
  for (std::uint8_t i = 0; i < D; ++i)
  {
    if (linal::isZero(rayDir[i]))
    {
      // If the ray direction component is zero, the ray is parallel to this axis
      // Set the inverse direction to infinity to handle divisions appropriately
      invDir[i] = std::numeric_limits<TFloat>::infinity();
    }
    else
    {
      // Compute the inverse of the direction component
      invDir[i] = TFloat{1} / rayDir[i];
    }
  }

  // Initialize vectors to hold the entry and exit distances (t-values) along each axis
  vec_type tMinVec;
  vec_type tMaxVec;

  // Get the minimum and maximum corners of the AABB
  const vec_type aabbMin = aabb.get_min();
  const vec_type aabbMax = aabb.get_max();

  // For each axis, compute the t-values where the ray intersects the slab boundaries
  for (std::uint8_t i = 0; i < D; ++i)
  {
    if (linal::isZero(rayDir[i]))
    {
      // Ray is parallel to the slabs on this axis
      // If the origin is outside the slab, there is no intersection
      if (linal::isLess(rayOrigin[i], aabbMin[i]) || linal::isGreater(rayOrigin[i], aabbMax[i]))
      {
        // The ray is outside the slab and parallel to it, so no intersection occurs
        return {};
      }
      else
      {
        // The ray is inside the slab, so set t-values to negative and positive infinity
        tMinVec[i] = -std::numeric_limits<TFloat>::infinity();
        tMaxVec[i] = std::numeric_limits<TFloat>::infinity();
      }
    }
    else
    {
      // Calculate the t-values where the ray intersects the slab boundaries
      // t1 and t2 are the distances along the ray to the slab planes
      TFloat t1 = (aabbMin[i] - rayOrigin[i]) * invDir[i];
      TFloat t2 = (aabbMax[i] - rayOrigin[i]) * invDir[i];

      // Ensure tMinVec[i] is the entry point and tMaxVec[i] is the exit point along this axis
      tMinVec[i] = std::min(t1, t2);
      tMaxVec[i] = std::max(t1, t2);
    }
  }

  // Compute tEnter and tExit for the ray's intersection with the AABB
  // tEnter is the largest entry point among all axes
  // tExit is the smallest exit point among all axes
  TFloat tEnter = std::max({tMinVec[0], tMinVec[1], tMinVec[2]});
  TFloat tExit = std::min({tMaxVec[0], tMaxVec[1], tMaxVec[2]});

  return {tEnter, tExit};
}

template <typename TFloat>
GEO_NODISCARD constexpr linal::vec<TFloat, 3> ordinate_to_point(const Ray<TFloat, 3>& ray, TFloat t) noexcept
{
  GEO_ASSERT(linal::isGreaterEq(t, TFloat{0}));
  return ray.get_origin() + t * ray.get_direction();
}

} // namespace details

} // namespace Geometry

#endif // GEOMETRY_INTERSECTAABBHELPER_HPP
