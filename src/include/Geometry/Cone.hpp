#ifndef GEOMETRY_CONE_HPP
#define GEOMETRY_CONE_HPP

#include "FwdGeometry.hpp"
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
class Cone
{
    Geometry::Segment3<T> m_segment;
    T m_radius;

  public:
    Cone(const Geometry::Segment3<T>& segment, const T& radius);

    [[nodiscard]] const Geometry::Segment3<T>& getSegment() const;
    [[nodiscard]] const T& getRadius() const;

    bool operator==(const Cone& rhs) const;
    bool operator!=(const Cone& rhs) const;
};
Cone::Cone(const Geometry::Segment3<T>& segment, const T& radius)
    : m_segment(segment), m_radius(radius)
{
}
const Geometry::Segment3<T>& Cone::getSegment() const
{
    return m_segment;
}
const T& Cone::getRadius() const
{
    return m_radius;
}
bool Cone::operator==(const Cone& rhs) const
{
    return m_segment == rhs.m_segment && m_radius == rhs.m_radius;
}
bool Cone::operator!=(const Cone& rhs) const
{
    return !(rhs == *this);
}
} // namespace Geometry

#endif // GEOMETRY_CONE_HPP
