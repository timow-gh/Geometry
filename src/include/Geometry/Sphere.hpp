#ifndef GLFWTESTAPP_SPHERE_H
#define GLFWTESTAPP_SPHERE_H

#include <Core/Math/Eps.hpp>
#include <Geometry/Lcs.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T>
class Sphere {
    LinAl::Vec3<T> m_origin;
    T m_radius;

  public:
    CORE_CONSTEXPR Sphere(const LinAl::Vec3<T>& origin, T radius) : m_origin(origin), m_radius(radius) {}

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const { return m_origin; }

    CORE_NODISCARD CORE_CONSTEXPR T getRadius() const { return m_radius; }

    CORE_CONSTEXPR void setOrigin(const LinAl::Vec3<T>& origin) { m_origin = origin; }

    CORE_CONSTEXPR void setRadius(T radius) { m_radius = radius; }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<T>& vec) const
    {
        if (Core::isGreater(LinAl::norm2(LinAl::Vec3<T>{vec - m_origin}), m_radius))
            return false;
        return true;
    }

    CORE_CONSTEXPR bool operator==(const Sphere& rhs) const { return m_origin == rhs.m_origin && m_radius == rhs.m_radius; }

    CORE_CONSTEXPR bool operator!=(const Sphere& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_SPHERE_H
