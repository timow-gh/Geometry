#ifndef GLFWTESTAPP_CYLINDER_HPP
#define GLFWTESTAPP_CYLINDER_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/FwdGeometry.hpp>
#include <Geometry/Lcs.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Cylinder {
    Segment3<T> m_segment;
    T m_radius;

  public:
    CORE_CONSTEXPR Cylinder(const Segment3<T>& segment, T radius)
        : m_segment(segment), m_radius(radius)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const Segment3<T>& getSegment() const
    {
        return m_segment;
    }

    CORE_NODISCARD CORE_CONSTEXPR T getRadius() const
    {
        return m_radius;
    }

    CORE_NODISCARD CORE_CONSTEXPR T height() const
    {
        return m_segment.length();
    }

    CORE_CONSTEXPR bool operator==(const Cylinder& rhs) const
    {
        return m_segment == rhs.m_segment && m_radius == rhs.m_radius;
    }

    CORE_CONSTEXPR bool operator!=(const Cylinder& rhs) const
    {
        return !(rhs == *this);
    }
};

template <typename T>
Cylinder<T> transformation(const Cylinder<T>& cylinder, const LinAl::HMatrix<T>& trafo)
{
    return Cylinder<T>{transformation(cylinder.getSegment(), trafo), cylinder.getRadius()};
}

} // namespace Geometry

#endif // GLFWTESTAPP_CYLINDER_HPP
