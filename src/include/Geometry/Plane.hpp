#ifndef GLFWTESTAPP_PLANE_HPP
#define GLFWTESTAPP_PLANE_HPP

#include "Core/Math/Eps.hpp"
#include "Geometry/Distance/DistancePlane.hpp"
#include "Geometry/Intersection/IntersectionPlane.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "Line.hpp"
#include "Ray.hpp"

namespace Geometry
{
template <typename T>
class Plane
{
  private:
    LinAl::Vec3<T> m_origin;
    LinAl::Vec3<T> m_normal;

  public:
    Plane(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& normal);

    [[nodiscard]] const LinAl::Vec3<T>& getOrigin() const;
    [[nodiscard]] const LinAl::Vec3<T>& getNormal() const;

    [[nodiscard]] T distance(const LinAl::Vec3<T>& vec) const;

    [[nodiscard]] std::optional<LinAl::Vec3<T>>
    intersection(const Line3<T>& line) const;
    [[nodiscard]] std::optional<LinAl::Vec3<T>>
    intersection(const Ray3<T>& ray) const;
    [[nodiscard]] std::optional<LinAl::Vec3<T>>
    intersection(const Segment3<T>& seg) const;
};

template <typename T>
Plane<T>::Plane(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& normal)
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
