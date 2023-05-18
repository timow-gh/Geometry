#ifndef GLFWTESTAPP_SEGMENT_H
#define GLFWTESTAPP_SEGMENT_H

#include <Core/Math/Eps.hpp>
#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Segment {
  linal::Vec<T, D> m_source;
  linal::Vec<T, D> m_target;

public:
  GEO_CONSTEXPR Segment() = default;

  GEO_CONSTEXPR Segment(linal::Vec<T, D> source, linal::Vec<T, D> target)
      : m_source(source)
      , m_target(target)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::Vec<T, D> getSource() const { return m_source; }
  GEO_NODISCARD GEO_CONSTEXPR linal::Vec<T, D> getTarget() const { return m_target; }

  GEO_CONSTEXPR void setSource(linal::Vec<T, D> source) { m_source = source; }
  GEO_CONSTEXPR void setTarget(linal::Vec<T, D> target) { m_target = target; }

  GEO_NODISCARD GEO_CONSTEXPR T length() const { return linal::norm2(linal::Vec<T, D>{m_target - m_source}); }

  //! Returns vector of length one.
  GEO_NODISCARD GEO_CONSTEXPR linal::Vec<T, D> direction() const
  {
    linal::Vec<T, D> segVec{m_target - m_source};
    return segVec / linal::norm2(segVec);
  }

  GEO_CONSTEXPR bool operator==(const Segment& rhs) const { return m_source == rhs.m_source && m_target == rhs.m_target; }
  GEO_CONSTEXPR bool operator!=(const Segment& rhs) const { return !(rhs == *this); }
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

#endif // GLFWTESTAPP_SEGMENT_H
