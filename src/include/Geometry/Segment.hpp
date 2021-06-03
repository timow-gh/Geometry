#ifndef GLFWTESTAPP_SEGMENT_H
#define GLFWTESTAPP_SEGMENT_H

#include "Core/Math/Eps.hpp"
#include "Geometry/Distance/DistanceSegment.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{

template <typename T, std::size_t D>
class Segment
{
    LinAl::Vec<T, D> m_source;
    LinAl::Vec<T, D> m_target;

  public:
    Segment() = default;
    Segment(const LinAl::Vec<T, D>& source, const LinAl::Vec<T, D>& target);

    [[nodiscard]] const LinAl::Vec<T, D>& getSource() const;
    [[nodiscard]] const LinAl::Vec<T, D>& getTarget() const;

    void setSource(const LinAl::Vec<T, D>& source);
    void setTarget(const LinAl::Vec<T, D>& target);

    [[nodiscard]] T distance(const LinAl::Vec<T, D>& vec) const;

    bool operator==(const Segment& rhs) const;
    bool operator!=(const Segment& rhs) const;
};

template <typename T, std::size_t D>
Segment<T, D>::Segment(const LinAl::Vec<T, D>& source,
                       const LinAl::Vec<T, D>& target)
    : m_source(source), m_target(target)
{
}
template <typename T, std::size_t D>
T Segment<T, D>::distance(const LinAl::Vec<T, D>& vec) const
{
    return Geometry::distance(*this, vec);
}

template <typename T, std::size_t D>
const LinAl::Vec<T, D>& Segment<T, D>::getSource() const
{
    return m_source;
}
template <typename T, std::size_t D>
const LinAl::Vec<T, D>& Segment<T, D>::getTarget() const
{
    return m_target;
}

template <typename T, std::size_t D>
void Segment<T, D>::setSource(const LinAl::Vec<T, D>& source)
{
    m_source = source;
}
template <typename T, std::size_t D>
void Segment<T, D>::setTarget(const LinAl::Vec<T, D>& target)
{
    m_target = target;
}

template <typename T, std::size_t D>
bool Segment<T, D>::operator==(const Segment& rhs) const
{
    return m_source == rhs.m_source && m_target == rhs.m_target;
}
template <typename T, std::size_t D>
bool Segment<T, D>::operator!=(const Segment& rhs) const
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
