#ifndef GEOMETRY_SEGMENT_H
#define GEOMETRY_SEGMENT_H

#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Segment {
  linal::vec<T, D> m_source;
  linal::vec<T, D> m_target;

public:
  constexpr Segment() = default;

  constexpr Segment(linal::vec<T, D> source, linal::vec<T, D> target)
      : m_source(source)
      , m_target(target)
  {
  }

  GEO_NODISCARD constexpr linal::vec<T, D> get_source() const noexcept { return m_source; }
  GEO_NODISCARD constexpr linal::vec<T, D> get_target() const noexcept { return m_target; }

  constexpr void set_source(linal::vec<T, D> source) noexcept { m_source = source; }
  constexpr void set_target(linal::vec<T, D> target) noexcept { m_target = target; }

  GEO_NODISCARD constexpr T length() const noexcept { return linal::norm2(linal::vec<T, D>{m_target - m_source}); }

  //! Returns vector of length one.
  GEO_NODISCARD constexpr linal::vec<T, D> direction() const noexcept
  {
    linal::vec<T, D> segVec{m_target - m_source};
    return segVec / linal::norm2(segVec);
  }

  constexpr bool operator==(const Segment& rhs) const noexcept
  {
    return linal::is_equal(m_source, rhs.m_source) && linal::is_equal(m_target, rhs.m_target);
  }
  constexpr bool operator!=(const Segment& rhs) const noexcept { return !(rhs == *this); }
};

template <typename T>
using Segment3 = class Segment<T, 3>;

template <typename T>
using Segment2 = Segment<T, 2>;
using Segment2f = Segment2<float>;
using Segment2d = Segment2<double>;

template <typename T>
using Segment3 = Segment<T, 3>;
using Segment3f = Segment3<float>;
using Segment3d = Segment3<double>;

} // namespace Geometry

#endif // GEOMETRY_SEGMENT_H
