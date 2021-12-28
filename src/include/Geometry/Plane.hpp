#ifndef GLFWTESTAPP_PLANE_HPP
#define GLFWTESTAPP_PLANE_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Distance/DistancePlane.hpp>
#include <Geometry/Intersection/IntersectionPlane.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T>
class Plane {
  private:
    LinAl::Vec3<T> m_origin;
    LinAl::Vec3<T> m_normal;

  public:
    CORE_CONSTEXPR Plane(const LinAl::Vec3<T>& origin,
                         const LinAl::Vec3<T>& normal);

    CORE_NODISCARD const LinAl::Vec3<T>& getOrigin() const;
    CORE_NODISCARD const LinAl::Vec3<T>& getNormal() const;

    CORE_NODISCARD T distance(const LinAl::Vec3<T>& vec) const;

    CORE_NODISCARD std::optional<LinAl::Vec3<T>>
    intersection(const Line3<T>& line) const;
    CORE_NODISCARD std::optional<LinAl::Vec3<T>>
    intersection(const Ray3<T>& ray) const;
    CORE_NODISCARD std::optional<LinAl::Vec3<T>>
    intersection(const Segment3<T>& seg) const;
};

template <typename T>
CORE_CONSTEXPR Plane<T>::Plane(const LinAl::Vec3<T>& origin,
                               const LinAl::Vec3<T>& normal)
    : m_origin(origin), m_normal(LinAl::normalize(normal))
{
}
template <typename T>
const LinAl::Vec3<T>& Plane<T>::getOrigin() const
{
    return m_origin;
}
template <typename T>
const LinAl::Vec3<T>& Plane<T>::getNormal() const
{
    return m_normal;
}
template <typename T>
T Plane<T>::distance(const LinAl::Vec3<T>& vec) const
{
    return Geometry::distance(*this, vec);
}
template <typename T>
std::optional<LinAl::Vec3<T>>
Plane<T>::intersection(const Geometry::Line3<T>& line) const
{
    return Geometry::calcIntersection(*this, line);
}
template <typename T>
std::optional<LinAl::Vec3<T>> Plane<T>::intersection(const Ray3<T>& ray) const
{
    return Geometry::calcIntersection(*this, ray);
}
template <typename T>
std::optional<LinAl::Vec3<T>>
Plane<T>::intersection(const Segment3<T>& seg) const
{
    return Geometry::calcIntersection(*this, seg);
}
} // namespace Geometry

#endif // GLFWTESTAPP_PLANE_HPP
