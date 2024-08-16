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
public:
  using MeshTraits_t = MeshTraits<TFloat, TIndex>;
  using HalfedgeMesh_t = HalfedgeMesh<MeshTraits_t>;
  using value_type = typename HalfedgeMesh_t::value_type;
  using index_type = typename HalfedgeMesh_t::index_type;

  using Cuboid_t = Cuboid<TFloat>;

private:
  std::optional<Cuboid_t> m_cube;
  linal::vec3<value_type> m_origin;
  linal::vec3<value_type> m_diagonal;

public:
  CuboidMeshBuilder() = default;

  GEO_NODISCARD CuboidMeshBuilder& set_origin(const linal::vec3<value_type>& origin)
  {
    m_origin = origin;
    return *this;
  }

  GEO_NODISCARD CuboidMeshBuilder& set_diagonal(const linal::vec3<value_type>& diagonal)
  {
    m_diagonal = diagonal;
    return *this;
  }

  GEO_NODISCARD CuboidMeshBuilder& set_cuboid(const Cuboid_t& cuboid)
  {
    m_cube = cuboid;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build()
  {
    if (!m_cube)
    {
      m_cube = Cuboid_t{m_origin, m_diagonal};
    }

    GEO_ASSERT(m_cube);

    std::array<Triangle3<value_type>, 12> triangles = calc_cuboid_triangles();
    auto heMesh = std::make_unique<HalfedgeMesh_t>();
    MeshTriangleAdder<MeshTraits_t> triangleAdder{*heMesh};
    for (const auto& triangle: triangles)
    {
      triangleAdder(triangle);
    }
    MeshTriangleAdder<MeshTraits_t>::set_opposite_halfedges(*heMesh);
    GEO_ASSERT(heMesh->getVertices().size() == 8);
    GEO_ASSERT(heMesh->getHalfedges().size() == 36);
    return heMesh;
  }

private:
  std::array<Triangle<value_type, 3>, 12> calc_cuboid_triangles()
  {
    auto sides = m_cube->get_side_vectors();
    linal::double3 defaultOrigin{0};

    const linal::vec3<value_type>& vecX = sides[0];
    const linal::vec3<value_type>& vecY = sides[1];
    const linal::vec3<value_type>& vecZ = sides[2];

    std::array<Triangle3<value_type>, 12> triangles;

    linal::double3 cubeOrigin = m_cube->get_origin();
    linal::double3 diagYZ = vecY + vecZ;
    calc_cuboid_face_triangles(triangles, {defaultOrigin, vecZ, diagYZ, vecY}, cubeOrigin, vecX, 0);

    linal::double3 diagXZ = vecX + vecZ;
    calc_cuboid_face_triangles(triangles, {defaultOrigin, vecX, diagXZ, vecZ}, cubeOrigin, vecY, 4);

    linal::double3 diagXY = vecX + vecY;
    calc_cuboid_face_triangles(triangles, {defaultOrigin, vecY, diagXY, vecX}, cubeOrigin, vecZ, 8);

    return triangles;
  }

  void calc_cuboid_face_triangles(std::array<Triangle<value_type, 3>, 12>& triangles,
                                  std::array<linal::vec3<value_type>, 4> vectors,
                                  const linal::vec3<value_type>& origin,
                                  const linal::vec3<value_type>& translationVec,
                                  std::size_t inserindex_type)
  {
    for (auto& vec: vectors)
    {
      vec = vec + origin;
    }
    triangles[inserindex_type++] = Triangle<value_type, 3>(vectors[0], vectors[1], vectors[2]);
    triangles[inserindex_type++] = Triangle<value_type, 3>(vectors[2], vectors[3], vectors[0]);

    for (auto& vec: vectors)
    {
      vec = vec + translationVec;
    }
    triangles[inserindex_type++] = Triangle<value_type, 3>(vectors[2], vectors[1], vectors[0]);
    triangles[inserindex_type] = Triangle<value_type, 3>(vectors[0], vectors[3], vectors[2]);
  }
};

} // namespace Geometry

#endif // GEOMETRY_CUBOIDMESHBUILDER_HPP
