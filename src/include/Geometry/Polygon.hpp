#ifndef GLFWTESTAPP_POLYGON_HPP
#define GLFWTESTAPP_POLYGON_HPP

#include "LinAl/LinearAlgebra.hpp"

namespace Geometry {
template <typename T, std::size_t D> class Polygon {
  LinAl::VecVector<T, D> m_points;

public:
  explicit Polygon(const LinAl::VecVector<T, D> &points);

  bool operator==(const Polygon &rhs) const;
  bool operator!=(const Polygon &rhs) const;

  const LinAl::VecVector<T, D> &getPoints() const;
};

template <typename T, std::size_t D>
Polygon<T, D>::Polygon(const LinAl::VecVector<T, D> &points)
    : m_points(points) {}

template <typename T, std::size_t D>
const LinAl::VecVector<T, D> &Polygon<T, D>::getPoints() const {
  return m_points;
}

template <typename T, std::size_t D>
bool Polygon<T, D>::operator==(const Polygon &rhs) const {
  return m_points == rhs.m_points;
}

template <typename T, std::size_t D>
bool Polygon<T, D>::operator!=(const Polygon &rhs) const {
  return !(rhs == *this);
}

} // namespace Geometry

#endif // GLFWTESTAPP_POLYGON_HPP
