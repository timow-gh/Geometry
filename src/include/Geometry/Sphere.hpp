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
    CORE_CONSTEXPR Sphere(const LinAl::Vec3<T>& origin, T radius);

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const;
    CORE_CONSTEXPR T getRadius() const;

    CORE_CONSTEXPR void setOrigin(const LinAl::Vec3<T>& origin);
    CORE_CONSTEXPR void setRadius(T radius);

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<T>& vec) const;

    CORE_CONSTEXPR bool operator==(const Sphere& rhs) const;
    CORE_CONSTEXPR bool operator!=(const Sphere& rhs) const;
};

template <typename T>
CORE_CONSTEXPR Sphere<T>::Sphere(const LinAl::Vec3<T>& origin, T radius)
    : m_origin(origin), m_radius(radius)
{
}

template <typename T>
CORE_CONSTEXPR const LinAl::Vec3<T>& Sphere<T>::getOrigin() const
{
    return m_origin;
}

template <typename T>
CORE_CONSTEXPR T Sphere<T>::getRadius() const
{
    return m_radius;
}

template <typename T>
CORE_CONSTEXPR void Sphere<T>::setOrigin(const LinAl::Vec3<T>& origin)
{
    m_origin = origin;
    m_origin = origin;
}

template <typename T>
CORE_CONSTEXPR void Sphere<T>::setRadius(T radius)
{
    m_radius = radius;
}

template <typename T>
CORE_CONSTEXPR bool Sphere<T>::contains(const LinAl::Vec3<T>& point) const
{
    if (Core::isGreater(LinAl::norm2(LinAl::Vec3<T>{point - m_origin}), m_radius))
        return false;
    return true;
}

template <typename T>
CORE_CONSTEXPR bool Sphere<T>::operator==(const Sphere& rhs) const
{
    return m_origin == rhs.m_origin && m_radius == rhs.m_radius;
}

template <typename T>
CORE_CONSTEXPR bool Sphere<T>::operator!=(const Sphere& rhs) const
{
    return !(rhs == *this);
}

} // namespace Geometry

#endif // GLFWTESTAPP_SPHERE_H
