#ifndef GEOMETRY_SPHEREMESHBUILDER_HPP
#define GEOMETRY_SPHEREMESHBUILDER_HPP

#include <cmath>
#include <memory>

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

#include "HalfedgeMesh.hpp"

namespace Geometry {
template <typename T>
std::unique_ptr<HalfedgeMesh<T>> buildSphereMesh(const Sphere<T> &sphere) {
  constexpr T CHORD_RADIUS_RATIO = T(0.1);
  const T deltaAngle = std::asin(T(0.5) * CHORD_RADIUS_RATIO) * T(2);

  std::size_t thetaSteps = M_PI / deltaAngle;
  std::size_t phiSteps = 2 * M_PI / deltaAngle + 1;

  // TODO use TArray
  Core::TVector<Triangle<T, 3>> triangles;

  // TODO Build Angle classes and conversions
  // TODO Build spherical lcs system and conversion to cartesian lcs
  // TODO Learn about the math functions, do they use radiant or deg (prob
  // radiant)

  // TODO Build a function that create quaads and then triangles from the points
  // of two circles
  // TODO The function needs to create a triangle from the first and last points
  // of the vector/array, too

  const LinAl::Vec3<T> origin = sphere.getOrigin();
  const T radius = sphere.getRadius();

  Core::TVector<LinAl::Vec3Vector<T>> circles;

  for (std::size_t j{1}; j < thetaSteps; ++j) {
    LinAl::Vec3Vector<T> circle;

    const T theta = j * deltaAngle;
    const T sinTheta = std::sin(theta);
    const T cosTheta = std::cos(theta);

    for (std::size_t j{0}; j < phiSteps; ++j) {
      const T phi = j * deltaAngle;
      const T sinPhi = std::sin(phi);
      const T cosPhi = std::cos(phi);
      circle.push_back(LinAl::Vec3<T>{origin[0] + radius * sinTheta * cosPhi,
                                      origin[1] + radius * sinTheta * sinPhi,
                                      origin[2] + radius * cosTheta});
    }
    circles.push_back(circle);
  }

  for (std::size_t i{0}; i < circles.size(); i++) {
    const std::size_t circlePointsSize = circles[i].size();
    const LinAl::Vec3Vector<T> &circlePoints = circles[i];

    if (i == 0) {
      const LinAl::Vec3<T> northPole =
          sphere.getOrigin() + LinAl::Vec3<T>{0, 0, 1} * radius;

      for (std::size_t j{0}; j < circlePointsSize - 1; ++j) {
        triangles.push_back(
            Triangle<T, 3>(northPole, circlePoints[j], circlePoints[j + 1]));
      }
      triangles.push_back(Triangle<T, 3>(
          northPole, circlePoints[circlePointsSize - 1], circlePoints[0]));
    } else {
      const LinAl::Vec3Vector<T> &prevCirclePoints = circles[i - 1];

      for (std::size_t j{0}; j < circlePointsSize - 1; ++j) {
        triangles.push_back(Triangle<T, 3>(prevCirclePoints[j], circlePoints[j],
                                           circlePoints[j + 1]));
        triangles.push_back(Triangle<T, 3>(
            circlePoints[j + 1], prevCirclePoints[j + 1], prevCirclePoints[j]));
      }

      triangles.push_back(Triangle<T, 3>(prevCirclePoints[circlePointsSize - 1],
                                         circlePoints[circlePointsSize - 1],
                                         circlePoints[0]));
      triangles.push_back(
          Triangle<T, 3>(circlePoints[0], prevCirclePoints[0],
                         prevCirclePoints[circlePointsSize - 1]));

      if (i == circles.size() - 1) {
        const LinAl::Vec3<T> southPole =
            sphere.getOrigin() - LinAl::Vec3<T>{0, 0, 1} * radius;

        for (std::size_t j{0}; j < circlePointsSize - 1; ++j) {
          triangles.push_back(
              Triangle<T, 3>(southPole, circlePoints[j + 1], circlePoints[j]));
        }
        triangles.push_back(Triangle<T, 3>(southPole, circlePoints[0],
                                           circlePoints[circlePointsSize - 1]));
      }
    }
  }

  auto sphereMesh = HalfedgeMesh<T>::create();
  for (const auto &triangle : triangles)
    sphereMesh->addTriangle(triangle);
  return sphereMesh;
  return {};
}
} // namespace Geometry

#endif // GEOMETRY_SPHEREMESHBUILDER_HPP
