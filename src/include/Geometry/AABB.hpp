#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include <Geometry/ExtremePointsInDirection.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <array>
#include <linal/containers.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

/**
 * @brief Axis Aligned Bounding Box
 *
 * @tparam TFloat The type of the floating point numbers
 * @tparam D The dimension of the AABB
 */
template <typename TFloat, std::size_t D>
class AABB {
  linal::vec<TFloat, D> m_min;
  linal::vec<TFloat, D> m_max;

public:
  GEO_CONSTEXPR AABB() GEO_NOEXCEPT = default;
  GEO_CONSTEXPR AABB(linal::vec<TFloat, D> min, linal::vec<TFloat, D> max) GEO_NOEXCEPT
      : m_min{min}
      , m_max{max}
  {
    GEO_ASSERT(m_min[0] <= m_max[0]);
    GEO_ASSERT(m_min[1] <= m_max[1]);
    GEO_ASSERT(m_min[2] <= m_max[2]);
  }

  GEO_CONSTEXPR AABB(linal::vec<TFloat, D> min, TFloat extend) GEO_NOEXCEPT : m_min{min}
  {
    for (std::size_t i = 0; i < D; ++i)
    {
      m_max[i] = min[i] + extend;
    }
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> get_min() const GEO_NOEXCEPT { return m_min; }
  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> get_max() const GEO_NOEXCEPT { return m_max; }

  GEO_CONSTEXPR void set_min(linal::vec<TFloat, D> min) GEO_NOEXCEPT { m_min = min; }
  GEO_CONSTEXPR void set_max(linal::vec<TFloat, D> max) GEO_NOEXCEPT { m_max = max; }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> get_center() const GEO_NOEXCEPT { return (m_min + (m_max - m_min) / TFloat{2}); }

  /**
   * @brief Check if the AABB is empty
   *
   * @return true, if the extend of the AABB is zero in any direction
   */
  GEO_NODISCARD GEO_CONSTEXPR bool is_empty() const GEO_NOEXCEPT
  {
    for (std::size_t i = 0; i < D; ++i)
    {
      if (m_min[i] == m_max[i])
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
  void add(linal::vec<TFloat, D> vec) GEO_NOEXCEPT
  {
    for (std::size_t i = 0; i < D; ++i)
    {
      if (vec[i] < m_min[i])
      {
        m_min[i] = vec[i];
      }
      if (vec[i] > m_max[i])
      {
        m_max[i] = vec[i];
      }
    }
  }

  /** @brief Grows the AABB to include the given AABB
   *
   * @param other The AABB to add
   */
  GEO_CONSTEXPR void add(const AABB& other) GEO_NOEXCEPT
  {
    for (std::size_t i = 0; i < D; ++i)
    {
      if (other.m_min[i] < m_min[i])
      {
        m_min[i] = other.m_min[i];
      }
      if (other.m_max[i] > m_max[i])
      {
        m_max[i] = other.m_max[i];
      }
    }
  }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const AABB<TFloat, D>& rhs) const { return m_min == rhs.m_min && m_max == rhs.m_max; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const AABB<TFloat, D>& rhs) const { return !(*this == rhs); }
};

template <typename TFloat, std::size_t D>
AABB<TFloat, D> makeAABB(const linal::vecvector<TFloat, D>& points)
{
  GEO_ASSERT(!points.empty());

  linal::vecArray<TFloat, D, D> axis;
  for (std::size_t i = 0; i < D; ++i)
  {
    axis[i][i] = 1;
  }

  std::array<MinMax<TFloat>, D> minMaxes;
  for (std::size_t i = 0; i < D; ++i)
  {
    minMaxes[i] = extremePointsAlongDirection(axis[i], points);
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
