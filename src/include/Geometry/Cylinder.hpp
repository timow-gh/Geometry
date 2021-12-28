#ifndef GLFWTESTAPP_CYLINDER_HPP
#define GLFWTESTAPP_CYLINDER_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/FwdGeometry.hpp>
#include <Geometry/Lcs.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T>
class Cylinder {
    Segment3<T> m_segment;
    T m_radius;

  public:
    CORE_CONSTEXPR Cylinder(const Segment3<T>& segment, T radius);

    CORE_NODISCARD CORE_CONSTEXPR const Segment3<T>& getSegment() const;
    CORE_NODISCARD CORE_CONSTEXPR T getRadius() const;

    CORE_NODISCARD CORE_CONSTEXPR T height() const;

    CORE_CONSTEXPR bool operator==(const Cylinder& rhs) const;
    CORE_CONSTEXPR bool operator!=(const Cylinder& rhs) const;
};

template <typename T>
CORE_CONSTEXPR Cylinder<T>::Cylinder(const Segment3<T>& segment, T radius)
    : m_segment(segment), m_radius(radius)
{
}

template <typename T>
CORE_CONSTEXPR const Segment3<T>& Cylinder<T>::getSegment() const
{
    return m_segment;
}

template <typename T>
CORE_CONSTEXPR T Cylinder<T>::getRadius() const
{
    return m_radius;
}

template <typename T>
CORE_CONSTEXPR T Cylinder<T>::height() const
{
    return m_segment.length();
}

template <typename T>
CORE_CONSTEXPR bool Cylinder<T>::operator==(const Cylinder& rhs) const
{
    return m_segment == rhs.m_segment && m_radius == rhs.m_radius;
}

template <typename T>
CORE_CONSTEXPR bool Cylinder<T>::operator!=(const Cylinder& rhs) const
{
    return !(rhs == *this);
}

} // namespace Geometry

#endif // GLFWTESTAPP_CYLINDER_HPP
