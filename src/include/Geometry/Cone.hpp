#ifndef GEOMETRY_CONE_HPP
#define GEOMETRY_CONE_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T>
class Cone {
    Geometry::Segment3<T> m_segment;
    T m_radius;

  public:
    CORE_CONSTEXPR
    Cone(const LinAl::Vec3<T>& circleMidPoint, const LinAl::Vec3<T>& peak, const T& radius);
    CORE_CONSTEXPR Cone(const Segment3<T>& segment, const T& radius);

    CORE_NODISCARD CORE_CONSTEXPR const Geometry::Segment3<T>& getSegment() const;
    CORE_NODISCARD CORE_CONSTEXPR const T& getRadius() const;

    CORE_NODISCARD CORE_CONSTEXPR T height() const;
    CORE_NODISCARD CORE_CONSTEXPR T slantHeight() const;
    CORE_NODISCARD CORE_CONSTEXPR T openingAngle() const;

    CORE_CONSTEXPR bool operator==(const Cone& rhs) const;
    CORE_CONSTEXPR bool operator!=(const Cone& rhs) const;
};

template <typename T>
CORE_CONSTEXPR
Cone<T>::Cone(const LinAl::Vec3<T>& circleMidPoint, const LinAl::Vec3<T>& peak, const T& radius)
    : m_segment(Segment3<T>{circleMidPoint, peak}), m_radius(radius)
{
}

template <typename T>
CORE_CONSTEXPR Cone<T>::Cone(const Geometry::Segment3<T>& segment, const T& radius)
    : m_segment(segment), m_radius(radius)
{
}

template <typename T>
CORE_CONSTEXPR const Geometry::Segment3<T>& Cone<T>::getSegment() const
{
    return m_segment;
}

template <typename T>
CORE_CONSTEXPR const T& Cone<T>::getRadius() const
{
    return m_radius;
}

template <typename T>
CORE_CONSTEXPR T Cone<T>::height() const
{
    return m_segment.length();
}

template <typename T>
CORE_CONSTEXPR T Cone<T>::slantHeight() const
{
    T h = height();
    return std::sqrt(m_radius * m_radius + h * h);
}

template <typename T>
CORE_CONSTEXPR T Cone<T>::openingAngle() const
{
    return std::atan(m_radius / height()) * 2;
}

template <typename T>
CORE_CONSTEXPR bool Cone<T>::operator==(const Cone<T>& rhs) const
{
    return m_segment == rhs.m_segment && m_radius == rhs.m_radius;
}

template <typename T>
CORE_CONSTEXPR bool Cone<T>::operator!=(const Cone<T>& rhs) const
{
    return !(rhs == *this);
}

} // namespace Geometry

#endif // GEOMETRY_CONE_HPP
