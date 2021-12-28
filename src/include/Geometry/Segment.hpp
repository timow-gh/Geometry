#ifndef GLFWTESTAPP_SEGMENT_H
#define GLFWTESTAPP_SEGMENT_H

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Distance/DistanceSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Segment {
    LinAl::Vec<T, D> m_source;
    LinAl::Vec<T, D> m_target;

  public:
    CORE_CONSTEXPR Segment() = default;
    CORE_CONSTEXPR Segment(const LinAl::Vec<T, D>& source, const LinAl::Vec<T, D>& target);

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getSource() const;
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getTarget() const;

    CORE_CONSTEXPR void setSource(const LinAl::Vec<T, D>& source);
    CORE_CONSTEXPR void setTarget(const LinAl::Vec<T, D>& target);

    CORE_NODISCARD CORE_CONSTEXPR T distance(const LinAl::Vec<T, D>& vec) const;

    CORE_NODISCARD CORE_CONSTEXPR T length() const;
    //! Returns vector of length one.
    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec<T, D> direction() const;

    CORE_CONSTEXPR bool operator==(const Segment& rhs) const;
    CORE_CONSTEXPR bool operator!=(const Segment& rhs) const;
};

template <typename T, std::size_t D>
CORE_CONSTEXPR Segment<T, D>::Segment(const LinAl::Vec<T, D>& source,
                                      const LinAl::Vec<T, D>& target)
    : m_source(source), m_target(target)
{
}

template <typename T, std::size_t D>
CORE_CONSTEXPR const LinAl::Vec<T, D>& Segment<T, D>::getSource() const
{
    return m_source;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR const LinAl::Vec<T, D>& Segment<T, D>::getTarget() const
{
    return m_target;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR void Segment<T, D>::setSource(const LinAl::Vec<T, D>& source)
{
    m_source = source;
}
template <typename T, std::size_t D>
CORE_CONSTEXPR void Segment<T, D>::setTarget(const LinAl::Vec<T, D>& target)
{
    m_target = target;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR T Segment<T, D>::distance(const LinAl::Vec<T, D>& vec) const
{
    return Geometry::distance(*this, vec);
}

template <typename T, std::size_t D>
CORE_CONSTEXPR T Segment<T, D>::length() const
{
    return LinAl::norm2(LinAl::Vec<T, D>{m_target - m_source});
}

template <typename T, std::size_t D>
CORE_CONSTEXPR LinAl::Vec<T, D> Segment<T, D>::direction() const
{
    LinAl::Vec<T, D> segVec{m_target - m_source};
    return segVec / LinAl::norm2(segVec);
}

template <typename T, std::size_t D>
CORE_CONSTEXPR bool Segment<T, D>::operator==(const Segment& rhs) const
{
    return m_source == rhs.m_source && m_target == rhs.m_target;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR bool Segment<T, D>::operator!=(const Segment& rhs) const
{
    return !(rhs == *this);
}

template <typename T>
using Segment2 = Segment<T, 2>;
using Segment2f = Segment2<float_t>;
using Segment2d = Segment2<double_t>;

template <typename T>
using Segment3 = Segment<T, 3>;
using Segment3f = Segment3<float_t>;
using Segment3d = Segment3<double_t>;
} // namespace Geometry

#endif // GLFWTESTAPP_SEGMENT_H
