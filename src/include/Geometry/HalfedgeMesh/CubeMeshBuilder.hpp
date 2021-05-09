#ifndef GEOMETRY_CUBEMESHBUILDER_HPP
#define GEOMETRY_CUBEMESHBUILDER_HPP

#include <algorithm>

#include "Core/Types/TArray.hpp"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/Cube.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/Triangle.hpp"

namespace Geometry {

template <typename T>
void calcCubeFaceTriangles(Core::TArray<Triangle<T, 3>, 12> &triangles,
                           LinAl::Vec3Array<T, 4> vectors,
                           const LinAl::Vec3<T> &origin,
                           const LinAl::Vec3<T> &translationVec,
                           std::size_t insertIndex) {
  for (auto &vec : vectors)
    vec = vec + origin;
  triangles[insertIndex++] = Triangle<T, 3>(vectors[0], vectors[1], vectors[2]);
  triangles[insertIndex++] = Triangle<T, 3>(vectors[2], vectors[3], vectors[0]);

  for (auto &vec : vectors)
    vec = vec + translationVec;
  triangles[insertIndex++] = Triangle<T, 3>(vectors[2], vectors[1], vectors[0]);
  triangles[insertIndex] = Triangle<T, 3>(vectors[0], vectors[3], vectors[2]);
}

template <typename T>
std::unique_ptr<HalfedgeMesh<T>> buildCubeMesh(const Cube<T> &cube) {
  auto sides = cube.sideVectors();
  LinAl::Vec3d defaultOrigin(0);

  const LinAl::Vec3<T> &x = sides[0];
  const LinAl::Vec3<T> &y = sides[1];
  const LinAl::Vec3<T> &z = sides[2];

  Core::TArray<Triangle<T, 3>, 12> triangles;

  LinAl::Vec3d diag = y + z;
  calcCubeFaceTriangles(triangles, {defaultOrigin, z, diag, y}, cube.origin(),
                        x, 0);

  diag = x + z;
  calcCubeFaceTriangles(triangles, {defaultOrigin, x, diag, z}, cube.origin(),
                        y, 4);

  diag = x + y;
  calcCubeFaceTriangles(triangles, {defaultOrigin, y, diag, x}, cube.origin(),
                        z, 8);

  auto cubeMesh = HalfedgeMesh<T>::create();
  for (const auto &triangle : triangles)
    cubeMesh->addTriangle(triangle);
  return cubeMesh;
}

} // namespace Geometry

#endif // GEOMETRY_CUBEMESHBUILDER_HPP
