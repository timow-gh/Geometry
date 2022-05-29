#ifndef GLFWTESTAPP_LINE_H
#define GLFWTESTAPP_LINE_H

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Distance/DistanceLine.hpp>
#include <Geometry/Intersection/IntersectionPlane.hpp>
#include <Geometry/Intersection/IntersectionSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class Line {
    LinAl::Vec<T, D> m_origin;
    LinAl::Vec<T, D> m_direction;

  public:
    CORE_CONSTEXPR Line(const LinAl::Vec<T, D>& origin, const LinAl::Vec<T, D>& direction)
        : m_origin(origin), m_direction(LinAl::normalize(LinAl::Vec<T, D>{direction}))
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getOrigin() const { return m_origin; }
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getDirection() const { return m_direction; }

    CORE_NODISCARD CORE_CONSTEXPR T distance(const LinAl::Vec<T, D>& vec) const { return Geometry::distance(vec, *this); }
    CORE_NODISCARD CORE_CONSTEXPR T distance(const Line<T, D>& rhs) const { return Geometry::distance(*this, rhs); }

    CORE_NODISCARD CORE_CONSTEXPR std::optional<LinAl::Vec3<T>> intersection(const Plane<T>& plane) const
    {
        return Geometry::calcIntersection(plane, *this);
    }
};

template <typename T>
using Line2 = Line<T, 2>;
using Line2f = Line2<float_t>;
using Line2d = Line2<double_t>;

template <typename T>
using Line3 = Line<T, 3>;
using Line3f = Line3<float_t>;
using Line3d = Line3<double_t>;
} // namespace Geometry

#endif // GLFWTESTAPP_LINE_H
