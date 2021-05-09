#ifndef GLFWTESTAPP_LINE_H
#define GLFWTESTAPP_LINE_H

#include "Core/Math/Eps.hpp"

#include <LinAl/LinearAlgebra.hpp>

#include "DistanceLine.hpp"
#include "IntersectionPlane.hpp"
#include "IntersectionSegment.hpp"

namespace Geometry {
template <typename T, std::size_t D> class Line {
  LinAl::Vec<T, D> m_origin;
  LinAl::Vec<T, D> m_direction;

public:
  Line(const LinAl::Vec<T, D> &origin, const LinAl::Vec<T, D> &direction);

  [[nodiscard]] const LinAl::Vec<T, D> &getOrigin() const;
  [[nodiscard]] const LinAl::Vec<T, D> &getDirection() const;

  [[nodiscard]] T distance(const LinAl::Vec<T, D> &vec) const;
  [[nodiscard]] T distance(const Line<T, D> &rhs) const;

  [[nodiscard]] std::optional<LinAl::Vec3<T>>
  intersection(const Plane<T> &plane) const;
};

template <typename T, std::size_t D>
Line<T, D>::Line(const LinAl::Vec<T, D> &origin,
                 const LinAl::Vec<T, D> &direction)
    : m_origin(origin),
      m_direction(LinAl::normalize(LinAl::Vec<T, D>{direction})) {}

template <typename T, std::size_t D>
const LinAl::Vec<T, D> &Line<T, D>::getOrigin() const {
  return m_origin;
}
template <typename T, std::size_t D>
const LinAl::Vec<T, D> &Line<T, D>::getDirection() const {
  return m_direction;
}
template <typename T, std::size_t D>
T Line<T, D>::distance(const LinAl::Vec<T, D> &vec) const {
  return Geometry::distance(vec, *this);
}
template <typename T, std::size_t D>
T Line<T, D>::distance(const Line<T, D> &rhs) const {
  return Geometry::distance(*this, rhs);
}
template <typename T, std::size_t D>
std::optional<LinAl::Vec3<T>>
Line<T, D>::intersection(const Plane<T> &plane) const {
  return Geometry::calcIntersection(plane, *this);
}

template <typename T> using Line2 = Line<T, 2>;
using Line2f = Line2<float_t>;
using Line2d = Line2<double_t>;

template <typename T> using Line3 = Line<T, 3>;
using Line3f = Line3<float_t>;
using Line3d = Line3<double_t>;
} // namespace Geometry

#endif // GLFWTESTAPP_LINE_H
