#ifndef GEOMETRY_CUBOIDMESHBUILDER_HPP
#define GEOMETRY_CUBOIDMESHBUILDER_HPP

#include "Geometry/Cuboid.hpp"
#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp"
#include "Geometry/Triangle.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <array>
#include <linal/vec.hpp>
#include <memory>
#include <optional>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class CuboidMeshBuilder {
  std::optional<Cuboid<TFloat>> m_cube;
  linal::vec3<TFloat> m_origin;
  linal::vec3<TFloat> m_diagonal;

public:
  CuboidMeshBuilder() = default;

  GEO_NODISCARD CuboidMeshBuilder& set_origin(const linal::vec3<TFloat>& origin)
  {
    m_origin = origin;
    return *this;
  }

  GEO_NODISCARD CuboidMeshBuilder& set_diagonal(const linal::vec3<TFloat>& diagonal)
  {
    m_diagonal = diagonal;
    return *this;
  }

  GEO_NODISCARD CuboidMeshBuilder& set_cuboid(const Cuboid<TFloat>& cuboid)
  {
    m_cube = cuboid;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build()
  {
    if (!m_cube)
    {
      m_cube = Cuboid<TFloat>{m_origin, m_diagonal};
    }

    GEO_ASSERT(m_cube);

    std::array<Triangle3<TFloat>, 12> triangles = calc_cuboid_triangles();
    auto heMesh = std::make_unique<HalfedgeMesh<TFloat, TIndex>>();
    std::for_each(triangles.begin(), triangles.end(), MeshTriangleAdder<TFloat, TIndex>{*heMesh});
    MeshTriangleAdder<TFloat, TIndex>::set_opposite_halfedges(*heMesh);
    GEO_ASSERT(heMesh->getVertices().size() == 8);
    GEO_ASSERT(heMesh->getHalfedges().size() == 36);
    return heMesh;
  }

private:
  std::array<Triangle<TFloat, 3>, 12> calc_cuboid_triangles()
  {
    auto sides = m_cube->get_side_vectors();
    linal::double3 defaultOrigin{0};

    const linal::vec3<TFloat>& x = sides[0];
    const linal::vec3<TFloat>& y = sides[1];
    const linal::vec3<TFloat>& z = sides[2];

    std::array<Triangle3<TFloat>, 12> triangles;

    linal::double3 diag = y + z;
    calc_cuboid_face_triangles(triangles, {defaultOrigin, z, diag, y}, m_cube->get_origin(), x, 0);

    diag = x + z;
    calc_cuboid_face_triangles(triangles, {defaultOrigin, x, diag, z}, m_cube->get_origin(), y, 4);

    diag = x + y;
    calc_cuboid_face_triangles(triangles, {defaultOrigin, y, diag, x}, m_cube->get_origin(), z, 8);

    return triangles;
  }

  void calc_cuboid_face_triangles(std::array<Triangle<TFloat, 3>, 12>& triangles,
                                  std::array<linal::vec3<TFloat>, 4> vectors,
                                  const linal::vec3<TFloat>& origin,
                                  const linal::vec3<TFloat>& translationVec,
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
