#ifndef GLFWTESTAPP_CYLINDER_HPP
#define GLFWTESTAPP_CYLINDER_HPP

#include "FwdGeometry.hpp"
#include "Geometry/Lcs.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{
template <typename T>
class Cylinder
{
    Segment3<T> m_segment;
    T m_radius;

  public:
    constexpr Cylinder(const Segment3<T>& segment, T radius);

    [[nodiscard]] const Segment3<T>& getSegment() const;
    [[nodiscard]] T getRadius() const;

    [[nodiscard]] T height() const;

    bool operator==(const Cylinder& rhs) const;
    bool operator!=(const Cylinder& rhs) const;
};

template <typename T>
constexpr Cylinder<T>::Cylinder(const Segment3<T>& segment, T radius)
    : m_segment(segment), m_radius(radius)
{
}

template <typename T>
const Segment3<T>& Cylinder<T>::getSegment() const
{
    return m_segment;
}

template <typename T>
T Cylinder<T>::getRadius() const
{
    return m_radius;
}

template <typename T>
T Cylinder<T>::height() const
{
    return m_segment.length();
}

template <typename T>
bool Cylinder<T>::operator==(const Cylinder& rhs) const
{
    return m_segment == rhs.m_segment && m_radius == rhs.m_radius;
}

template <typename T>
bool Cylinder<T>::operator!=(const Cylinder& rhs) const
{
    return !(rhs == *this);
}

} // namespace Geometry

#endif // GLFWTESTAPP_CYLINDER_HPP
