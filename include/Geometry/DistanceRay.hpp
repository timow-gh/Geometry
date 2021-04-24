#ifndef GEOMETRY_DISTANCERAY_HPP
#define GEOMETRY_DISTANCERAY_HPP

#include "Core/Math/Eps.hpp"

#include "LinAl/LinearAlgebra.hpp"

#include "Fwd.hpp"

namespace Geometry {
template <typename T, std::size_t D>
T distance(const Ray<T, D> &ray, const LinAl::Vec<T, D> &vec) {
  LinAl::Vec<T, D> vecO{vec - ray.getOrigin()};
  const LinAl::Vec<T, D> rayDirection = ray.getDirection();

  const T dotProduct = LinAl::dot(vecO, rayDirection);
  LinAl::Vec<T, D> distanceVec;
  if (Core::isGreater(dotProduct, T(0))) {
    distanceVec = LinAl::rejection(vecO, rayDirection);
  } else {
    distanceVec = ray.getOrigin() - vec;
  }

  return LinAl::norm2(distanceVec);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCERAY_HPP
