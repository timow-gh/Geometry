#ifndef GLFWTESTAPP_SPHERE_H
#define GLFWTESTAPP_SPHERE_H

#include "Lcs.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{
template <typename T>
class Sphere
{
    LinAl::Vec3<T> m_origin;
    T m_radius;

  public:
    Sphere(const LinAl::Vec3<T>& origin, T radius);

    const LinAl::Vec3<T>& getOrigin() const;
    T getRadius() const;

    bool operator==(const Sphere& rhs) const;
    bool operator!=(const Sphere& rhs) const;
};

template <typename T>
Sphere<T>::Sphere(const LinAl::Vec3<T>& origin, T radius)
    : m_origin(origin), m_radius(radius)
{
}
template <typename T>
const LinAl::Vec3<T>& Sphere<T>::getOrigin() const
{
    return m_origin;
}
template <typename T>
T Sphere<T>::getRadius() const
{
    return m_radius;
}
template <typename T>
bool Sphere<T>::operator==(const Sphere& rhs) const
{
    return m_origin == rhs.m_origin && m_radius == rhs.m_radius;
}
template <typename T>
bool Sphere<T>::operator!=(const Sphere& rhs) const
{
    return !(rhs == *this);
}

} // namespace Geometry

#endif // GLFWTESTAPP_SPHERE_H
