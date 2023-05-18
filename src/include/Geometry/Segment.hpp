#ifndef GLFWTESTAPP_SEGMENT_H
#define GLFWTESTAPP_SEGMENT_H

#include <Core/Math/Eps.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Fwd/FwdSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Segment {
  LinAl::Vec<T, D> m_source;
  LinAl::Vec<T, D> m_target;

public:
  GEO_CONSTEXPR Segment() = default;

  GEO_CONSTEXPR Segment(LinAl::Vec<T, D> source, LinAl::Vec<T, D> target)
      : m_source(source)
      , m_target(target)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec<T, D> getSource() const { return m_source; }
  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec<T, D> getTarget() const { return m_target; }

  GEO_CONSTEXPR void setSource(LinAl::Vec<T, D> source) { m_source = source; }
  GEO_CONSTEXPR void setTarget(LinAl::Vec<T, D> target) { m_target = target; }

  GEO_NODISCARD GEO_CONSTEXPR T length() const { return LinAl::norm2(LinAl::Vec<T, D>{m_target - m_source}); }

  //! Returns vector of length one.
  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec<T, D> direction() const
  {
    LinAl::Vec<T, D> segVec{m_target - m_source};
    return segVec / LinAl::norm2(segVec);
  }

  GEO_CONSTEXPR bool operator==(const Segment& rhs) const { return m_source == rhs.m_source && m_target == rhs.m_target; }
  GEO_CONSTEXPR bool operator!=(const Segment& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_SEGMENT_H
