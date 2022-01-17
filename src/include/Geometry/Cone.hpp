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
    Cone(const LinAl::Vec3<T>& circleMidPoint, const LinAl::Vec3<T>& peak, const T& radius)
        : m_segment(Segment3<T>{circleMidPoint, peak}), m_radius(radius)
    {
    }

    CORE_CONSTEXPR Cone(const Segment3<T>& segment, const T& radius)
        : m_segment(segment), m_radius(radius)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const Geometry::Segment3<T>& getSegment() const
    {
        return m_segment;
    }

    CORE_NODISCARD CORE_CONSTEXPR const T& getRadius() const
    {
        return m_radius;
    }

    CORE_NODISCARD CORE_CONSTEXPR T height() const
    {
        return m_segment.length();
    }

    CORE_NODISCARD CORE_CONSTEXPR T slantHeight() const
    {
        T h = height();
        return std::sqrt(m_radius * m_radius + h * h);
    }

    CORE_NODISCARD CORE_CONSTEXPR T openingAngle() const
    {
        return std::atan(m_radius / height()) * 2;
    }

    CORE_CONSTEXPR bool operator==(const Cone& rhs) const
    {
        return m_segment == rhs.m_segment && m_radius == rhs.m_radius;
    }

    CORE_CONSTEXPR bool operator!=(const Cone& rhs) const
    {
        return !(rhs == *this);
    }
};

} // namespace Geometry

#endif // GEOMETRY_CONE_HPP
