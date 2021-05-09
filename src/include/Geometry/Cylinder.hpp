#ifndef GLFWTESTAPP_CYLINDER_HPP
#define GLFWTESTAPP_CYLINDER_HPP

#include "Geometry/Lcs.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry {
template <typename T> class Cylinder {
  Lcs<T> m_originLcs;
  T m_height;
  T m_radius;

public:
  Cylinder(const Lcs<T> &originLcs, T height, T radius);

  bool operator==(const Cylinder &rhs) const;
  bool operator!=(const Cylinder &rhs) const;

  const Lcs<T> &getOriginLcs() const;
  T getHeight() const;
  T getRadius() const;
};

template <typename T>
Cylinder<T>::Cylinder(const Lcs<T> &originLcs, T height, T radius)
    : m_originLcs(originLcs), m_height(height), m_radius(radius) {}

template <typename T> bool Cylinder<T>::operator==(const Cylinder &rhs) const {
  return m_originLcs == rhs.m_originLcs && m_height == rhs.m_height &&
         m_radius == rhs.m_radius;
}

template <typename T> bool Cylinder<T>::operator!=(const Cylinder &rhs) const {
  return !(rhs == *this);
}
template <typename T> const Lcs<T> &Cylinder<T>::getOriginLcs() const {
  return m_originLcs;
}
template <typename T> T Cylinder<T>::getHeight() const { return m_height; }
template <typename T> T Cylinder<T>::getRadius() const { return m_radius; }

} // namespace Geometry

#endif // GLFWTESTAPP_CYLINDER_HPP
