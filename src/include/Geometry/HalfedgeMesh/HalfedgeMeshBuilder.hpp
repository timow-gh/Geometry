#ifndef GLFWTESTAPP_MESHBUILDER_H
#define GLFWTESTAPP_MESHBUILDER_H

#include "CubeMeshBuilder.hpp"
#include "SphereMeshBuilder.hpp"

namespace Geometry {
class HalfedgeMeshBuilder {
public:
  template <typename T>
  [[nodiscard]] static std::unique_ptr<HalfedgeMesh<T>>
  buildMesh(const Cube<T> &cube) {
    return buildCubeMesh<T>(cube);
  }

  template <typename T>
  [[nodiscard]] static std::unique_ptr<HalfedgeMesh<T>>
  buildMesh(const Sphere<T> &sphere) {
    return buildSphereMesh<T>(sphere);
  }
};

} // namespace Geometry

#endif // GLFWTESTAPP_MESHBUILDER_H
