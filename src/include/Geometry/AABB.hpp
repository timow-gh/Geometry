#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include "Geometry/ExtremePointsInDirection.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>

namespace Geometry
{

/**
 * @brief Axis Aligned Bounding Box
 */
template <typename TFloat, std::uint8_t D>
class AABB {
public:
  using size_type = typename linal::vec<TFloat, D>::size_type;

private:
  linal::vec<TFloat, D> m_min;
  linal::vec<TFloat, D> m_max;

public:
  /** @brief Construct a new, invalid AABB  */
  constexpr AABB() noexcept
      : m_min{linal::vec<TFloat, D>{1.0}}
      , m_max{linal::vec<TFloat, D>{-1.0}}
  {
  }

  constexpr AABB(linal::vec<TFloat, D> min, linal::vec<TFloat, D> max) noexcept
      : m_min{min}
      , m_max{max}
  {
    GEO_ASSERT(is_valid());
  }

/** @brief Construct a new AABB from a min point and an (positive) extend
 *
 * @remark The extend must be positive
 *
 * @param min The minimum point of the AABB
 * @param extend The extend of the AABB in every positive direction
 */
constexpr AABB(linal::vec<TFloat, D> min, TFloat extend) noexcept
    : m_min{min}
{
  GEO_ASSERT(extend >= TFloat{0.0});
  for (size_type i = 0; i < static_cast<size_type>(D); ++i)
  {
    m_max[i] = min[i] + extend;
  }
}

  GEO_NODISCARD constexpr linal::vec<TFloat, D> get_min() const noexcept { return m_min; }
  GEO_NODISCARD constexpr linal::vec<TFloat, D> get_max() const noexcept { return m_max; }

  constexpr void set_min(linal::vec<TFloat, D> min) noexcept { m_min = min; }
  constexpr void set_max(linal::vec<TFloat, D> max) noexcept { m_max = max; }

  GEO_NODISCARD constexpr linal::vec<TFloat, D> center() const noexcept { return (m_min + (m_max - m_min) / TFloat{2}); }

  /** @brief Checks if min and max are valid
   *
   * @return true, if min <= max in every direction
   */
  GEO_NODISCARD constexpr bool is_valid() const noexcept
  {
    for (size_type i = 0; i < static_cast<size_type>(D); ++i)
    {
      if (linal::isGreaterEq(m_min[i], m_max[i]))
      {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check if the AABB is empty
   */
  GEO_NODISCARD constexpr bool is_empty() const noexcept
  {
    for (size_type i = 0; i < static_cast<size_type>(D); ++i)
    {
      if (linal::isGreater(m_min[i], m_max[i]))
      {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Grows the AABB to include the given point
   *
   * @param vec The point to add
   */
  void add(linal::vec<TFloat, D> vec) noexcept
  {
    for (size_type i = 0; i < D; ++i)
    {
      if (linal::isLess(vec[i], m_min[i]))
      {
        m_min[i] = vec[i];
      }
      if (linal::isGreater(vec[i], m_max[i]))
      {
        m_max[i] = vec[i];
      }
    }
  }

  /** @brief Grows the AABB to include the given AABB
   *
   * @param other The AABB to add
   */
  constexpr void add(const AABB& other) noexcept
  {
    for (size_type i = 0; i < D; ++i)
    {
      if (linal::isLess(other.m_min[i], m_min[i]))
      {
        m_min[i] = other.m_min[i];
      }
      if (linal::isGreater(other.m_max[i], m_max[i]))
      {
        m_max[i] = other.m_max[i];
      }
    }
  }

  GEO_NODISCARD constexpr bool operator==(const AABB<TFloat, D>& rhs) const noexcept
  {
    return linal::is_equal(m_min, rhs.m_min) && linal::is_equal(m_max, rhs.m_max);
  }
  GEO_NODISCARD constexpr bool operator!=(const AABB<TFloat, D>& rhs) const noexcept { return !(*this == rhs); }
};

/*@brief Creates an AABB from a set of points
 *
 * @tparam TFloat The type of the floating point numbers
 * @tparam D The dimension of the AABB
 * @param points The points to create the AABB from
 * @return The created AABB
 */
template <typename TFloat, std::uint8_t D>
GEO_NODISCARD auto make_aabb(const std::vector<linal::vec<TFloat, D>>& points) noexcept
{
  GEO_ASSERT(!points.empty());

  using size_type = typename linal::vec<TFloat, D>::size_type;

  std::array<linal::vec<TFloat, D>, D> axis;
  for (size_type i = 0; i < D; ++i)
  {
    axis[static_cast<std::size_t>(i)][i] = 1;
  }

  std::array<MinMax<TFloat>, D> minMaxes;
  for (size_type i = 0; i < D; ++i)
  {
    minMaxes[static_cast<std::size_t>(i)] = extreme_points_along_direction(axis[static_cast<std::size_t>(i)], points);
  }

  return AABB<TFloat, D>{linal::vec<TFloat, D>{minMaxes[0].min, minMaxes[1].min, minMaxes[2].min},
                         linal::vec<TFloat, D>{minMaxes[0].max, minMaxes[1].max, minMaxes[2].max}};
}

template <typename TFloat>
using AABB2 = AABB<TFloat, 2>;
using AABB2f = AABB2<float>;
using AABB2d = AABB2<double>;

template <typename TFloat>
using AABB3 = AABB<TFloat, 3>;
using AABB3f = AABB3<float>;
using AABB3d = AABB3<double>;

} // namespace Geometry

#endif // GEOMETRY_AABB_HPP
