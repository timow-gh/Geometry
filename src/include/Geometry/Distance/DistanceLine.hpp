#ifndef GEOMETRY_DISTANCELINE_HPP
#define GEOMETRY_DISTANCELINE_HPP

#include "Core/Math/Eps.hpp"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/Fwd.hpp"

namespace Geometry {
template <typename T, std::size_t D>
[[nodiscard]] T distance(const LinAl::Vec<T, D> &vec, const Line<T, D> &line) {
  return LinAl::norm2(LinAl::rejection(LinAl::Vec<T, D>{line.getOrigin() - vec},
                                       line.getDirection()));
}
template <typename T, std::size_t D>
[[nodiscard]] T distance(const Line<T, D> &lhs, const Line<T, D> &rhs) {
  LinAl::Vec<T, D> cross = LinAl::cross(lhs.getDirection(), rhs.getDirection());
  T crossLen = LinAl::norm2(cross);
  if (Core::isZero(crossLen))
    return distance(lhs.getOrigin(), rhs);
  LinAl::Vec<T, D> originToOrigin = lhs.getOrigin() - rhs.getOrigin();
  return LinAl::dot(originToOrigin, cross) / crossLen;
}
} // namespace Geometry

#endif // GEOMETRY_DISTANCELINE_HPP
