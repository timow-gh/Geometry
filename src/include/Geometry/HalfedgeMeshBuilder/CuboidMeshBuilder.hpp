#ifndef GEOMETRY_CUBOIDMESHBUILDER_HPP
#define GEOMETRY_CUBOIDMESHBUILDER_HPP

#include <array>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp>
#include <Geometry/Triangle.hpp>
#include <Geometry/Utils/Assert.hpp>
#include <linal/Vec3.hpp>
#include <algorithm>
#include <optional>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class CuboidMeshBuilder {
  std::optional<Cuboid<TFloat>> m_cube;

public:
  CuboidMeshBuilder() = default;
  CuboidMeshBuilder& setCuboid(const Cuboid<TFloat>& cube)
  {
    m_cube = cube;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build()
  {
    GEO_ASSERT(m_cube);

    if (!m_cube)
      return nullptr;

    std::array<Triangle<TFloat, 3>, 12> triangles = calcCuboidTriangles();
    auto heMesh = std::make_unique<HalfedgeMesh<TFloat, TIndex>>();
    std::for_each(triangles.cbegin(), triangles.cend(), MeshTriangleAdder<TFloat, TIndex>(*heMesh));
    return heMesh;
  }

private:
  std::array<Triangle<TFloat, 3>, 12> calcCuboidTriangles()
  {
    auto sides = m_cube->getSideVectors();
    linal::Vec3d defaultOrigin{0};

    const linal::Vec3<TFloat>& x = sides[0];
    const linal::Vec3<TFloat>& y = sides[1];
    const linal::Vec3<TFloat>& z = sides[2];

    std::array<Triangle<TFloat, 3>, 12> triangles;

    linal::Vec3d diag = y + z;
    calcCuboidFaceTriangles(triangles, {defaultOrigin, z, diag, y}, m_cube->getOrigin(), x, 0);

    diag = x + z;
    calcCuboidFaceTriangles(triangles, {defaultOrigin, x, diag, z}, m_cube->getOrigin(), y, 4);

    diag = x + y;
    calcCuboidFaceTriangles(triangles, {defaultOrigin, y, diag, x}, m_cube->getOrigin(), z, 8);

    return triangles;
  }

  void calcCuboidFaceTriangles(std::array<Triangle<TFloat, 3>, 12>& triangles,
                               linal::Vec3Array<TFloat, 4> vectors,
                               const linal::Vec3<TFloat>& origin,
                               const linal::Vec3<TFloat>& translationVec,
                               std::size_t insertIndex)
  {
    for (auto& vec: vectors)
      vec = vec + origin;
    triangles[insertIndex++] = Triangle<TFloat, 3>(vectors[0], vectors[1], vectors[2]);
    triangles[insertIndex++] = Triangle<TFloat, 3>(vectors[2], vectors[3], vectors[0]);

    for (auto& vec: vectors)
      vec = vec + translationVec;
    triangles[insertIndex++] = Triangle<TFloat, 3>(vectors[2], vectors[1], vectors[0]);
    triangles[insertIndex] = Triangle<TFloat, 3>(vectors[0], vectors[3], vectors[2]);
  }
};

} // namespace Geometry

#endif // GEOMETRY_CUBOIDMESHBUILDER_HPP
