#ifndef GEOMETRY_INTERSECTAABB_HPP
#define GEOMETRY_INTERSECTAABB_HPP

#include "Geometry/AABB.hpp"
#include "Geometry/Intersect/IntersectAABBHelper.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Utils/DefaultValues.hpp"
#include <limits>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

/** @brief Checks if two AABBs are intersecting.
 *
 * Two AABBs are intersecting if they have a non-empty intersection.
 *
 * @tparam TFloat The type of the AABB components.
 * @tparam D The dimension of the AABBs.
 * @param lhs The first AABB.
 * @param rhs The second AABB.
 * @return bool True if the AABBs are intersecting, false otherwise.
 */
template <typename TFloat, std::uint8_t D>
GEO_NODISCARD constexpr bool is_intersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs) noexcept
{
  auto lhsMax = lhs.get_max();
  auto lhsMin = lhs.get_min();
  auto rhsMax = rhs.get_max();
  auto rhsMin = rhs.get_min();

  using size_type = typename linal::vec<TFloat, D>::size_type;

  for (size_type i = 0; i < D; ++i)
  {
    if (linal::isLessEq(lhsMax[i], rhsMin[i]) || linal::isGreaterEq(lhsMin[i], rhsMax[i]))
    {
      return false;
    }
  }
  return true;
}

/** @brief Calculates the intersection of two AABBs.
 *
 * @tparam TFloat The type of the AABB components.
 * @tparam D The dimension of the AABBs.
 * @param lhs The first AABB.
 * @param rhs The second AABB.
 * @return AABB The intersection of the two AABBs or an invalid AABB if they do not intersect.
 */
template <typename TFloat, std::uint8_t D>
GEO_NODISCARD constexpr AABB<TFloat, D> intersect(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs) noexcept
{
  auto lhsMax = lhs.get_max();
  auto lhsMin = lhs.get_min();
  auto rhsMax = rhs.get_max();
  auto rhsMin = rhs.get_min();

  auto min = lhsMin;
  auto max = lhsMax;

  using size_type = typename linal::vec<TFloat, D>::size_type;

  for (size_type i = 0; i < D; ++i)
  {
    if (linal::isLessEq(lhsMax[i], rhsMin[i]) || linal::isGreaterEq(lhsMin[i], rhsMax[i]))
    {
      return AABB<TFloat, D>{};
    }
    min[i] = std::max(lhsMin[i], rhsMin[i]);
    max[i] = std::min(lhsMax[i], rhsMax[i]);
  }

  return AABB<TFloat, D>{min, max};
}

/** @brief Checks if a ray intersects an Axis-Aligned Bounding Box (AABB) in 3D space.
 *
 * This function implements the **slab method** for ray-AABB intersection testing.
 * The algorithm computes the entry and exit distances (`t` values) of the ray
 * as it intersects the slabs (the space between pairs of parallel planes) that define
 * the AABB along each of the three axes (x, y, z).
 *
 * By finding the maximum of the entry distances (`tEnter`) and the minimum of the exit
 * distances (`tExit`), we can determine if there is an overlapping interval where the
 * ray is inside all three slabs simultaneously. If such an interval exists and is valid
 * (i.e., `tEnter <= tExit` and `tExit >= 0`), the ray intersects the AABB.
 *
 * **Algorithm Steps:**
 * 1. **Compute Inverse Direction:** Calculate the inverse of each component of the ray's
 *    direction vector to optimize calculations and handle division efficiently.
 * 2. **Handle Parallel Rays:** If a ray is parallel to an axis (direction component is zero),
 *    check if the ray's origin is within the slab along that axis.
 * 3. **Calculate t-values:** For each axis, compute the `t` values where the ray intersects
 *    the slabs.
 * 4. **Find Entry and Exit Points:** Determine `tEnter` (maximum of entry `t` values) and
 *    `tExit` (minimum of exit `t` values).
 * 5. **Intersection Test:** If `tEnter <= tExit` and `tExit >= 0`, the ray intersects the AABB.
 *
 * **References:**
 * - *Real-Time Rendering* by Tomas Akenine-Möller et al.
 * - *"An Efficient and Robust Ray–Box Intersection Algorithm"* by Amy Williams et al.
 *
 * @tparam TFloat The floating-point type of the components (e.g., `float` or `double`).
 * @param ray The ray to test for intersection with the AABB.
 * @param aabb The axis-aligned bounding box to test against the ray.
 * @return `true` if the ray intersects the AABB, `false` otherwise.
 */
template <typename TFloat>
GEO_NODISCARD constexpr bool is_intersecting(const Ray<TFloat, 3>& ray, const AABB<TFloat, 3>& aabb) noexcept
{
  auto res = details::aabb_ray_intersection_interval(ray, aabb);
  auto& tEnter = res.tEnter;
  auto& tExit = res.tExit;

  // Check if the intervals are valid and the ray intersects the AABB
  // First condition: tEnter <= tExit ensures there is an overlapping interval
  // Second condition: tExit >= 0 ensures the intersection is in the ray's forward direction
  return linal::isLessEq(tEnter, tExit) && linal::isGreaterEq(tExit, TFloat{0});
}

/**
 * @brief Checks if a ray intersects with an axis-aligned bounding box (AABB).
 *
 * @copydoc is_intersecting(const Ray<TFloat, 3>&, const AABB<TFloat, 3>&)
 */
template <typename TFloat>
GEO_NODISCARD constexpr bool is_intersecting(const AABB<TFloat, 3>& aabb, const Ray<TFloat, 3>& ray) noexcept
{
  return is_intersecting(ray, aabb);
}

template <typename TFloat>
class AABBIntersectionResult {
  linal::vec<TFloat, 3> m_enter{vec3_highest_v<TFloat>};
  linal::vec<TFloat, 3> m_exit{vec3_lowest_v<TFloat>};
  bool m_hasEnter{false};
  bool m_hasExit{false};

public:
  constexpr AABBIntersectionResult() noexcept = default;
  constexpr AABBIntersectionResult(linal::vec<TFloat, 3> enter, linal::vec<TFloat, 3> exit) noexcept
      : m_enter(enter)
      , m_exit(exit)
      , m_hasEnter(true)
      , m_hasExit(true)
  {
  }

  GEO_NODISCARD constexpr linal::vec<TFloat, 3> get_enter() const noexcept { return m_enter; }
  GEO_NODISCARD constexpr linal::vec<TFloat, 3> get_exit() const noexcept { return m_exit; }

  GEO_NODISCARD constexpr bool has_enter() const noexcept { return m_hasEnter; }
  GEO_NODISCARD constexpr bool has_exit() const noexcept { return m_hasExit; }

  constexpr operator bool() const noexcept { return m_hasEnter || m_hasExit; }

  constexpr void set_enter(linal::vec<TFloat, 3> enter) noexcept
  {
    m_enter = enter;
    m_hasEnter = true;
  }
  constexpr void set_exit(linal::vec<TFloat, 3> exit) noexcept
  {
    m_exit = exit;
    m_hasExit = true;
  }
};

template <typename TFloat>
GEO_NODISCARD constexpr AABBIntersectionResult<TFloat> intersect(const Ray<TFloat, 3>& ray, const AABB<TFloat, 3>& aabb) noexcept
{
  auto intervalRes = details::aabb_ray_intersection_interval(ray, aabb);
  auto& tEnter = intervalRes.tEnter;
  auto& tExit = intervalRes.tExit;

  if (linal::isGreaterEq(tEnter, tExit))
  {
    return {};
  }

  AABBIntersectionResult<TFloat> result;
  if (linal::isGreaterEq(tEnter, TFloat{0}))
  {
    result.set_enter(details::ordinate_to_point(ray, tEnter));
  }
  if (linal::isGreaterEq(tExit, TFloat{0}))
  {
    result.set_exit(details::ordinate_to_point(ray, tExit));
  }
  return result;
}

template <typename TFloat>
GEO_NODISCARD constexpr AABBIntersectionResult<TFloat> intersect(const AABB<TFloat, 3>& aabb, const Ray<TFloat, 3>& ray) noexcept
{
  return intersect(ray, aabb);
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTAABB_HPP
