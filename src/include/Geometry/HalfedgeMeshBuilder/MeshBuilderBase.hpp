#ifndef GEOMETRY_MESHBUILDERBASE_HPP
#define GEOMETRY_MESHBUILDERBASE_HPP

#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp"
#include <linal/hmat.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <memory>
#include <vector>

namespace Geometry
{

template <typename TMeshTraits, typename Derived>
class MeshBuilderBase {
public:
  using value_type = typename TMeshTraits::value_type;
  using index_type = typename TMeshTraits::index_type;

  using HalfedgeMesh_t = typename TMeshTraits::HalfedgeMesh_t;

protected:
  linal::hmat<value_type> m_transformation = linal::hmat<value_type>::identity();

  std::unique_ptr<HalfedgeMesh_t> build_triangle_halfedge_mesh(const std::vector<linal::vec3<value_type>>& points,
                                                               const std::vector<index_type>& triangleIndices) const
  {
    GEO_ASSERT(points.size() > 0);
    GEO_ASSERT(triangleIndices.size() % 3 == 0);

    auto heMesh = std::make_unique<HalfedgeMesh_t>();
    MeshTriangleAdder<HalfedgeMesh_t> meshTriangleAdder{*heMesh};
    std::size_t size = triangleIndices.size();
    for (std::size_t i{2}; i < size; i += 3)
    {
      std::array<linal::vec3<value_type>, 3> trianglePoints;

      for (std::size_t j{0}; j < 3; ++j)
      {
        const std::size_t idx = triangleIndices[i - j];
        const linal::vec3<value_type>& point = points[idx];
        if (m_transformation != linal::hmat<value_type>::identity())
        {
          linal::hvec<value_type> tPoint = m_transformation * linal::hvec<value_type>{point[0], point[1], point[2], 1.0};
          trianglePoints[j] = linal::vec3<value_type>{tPoint[0], tPoint[1], tPoint[2]};
        }
        else
        {
          trianglePoints[j] = point;
        }
      }

      meshTriangleAdder(Triangle<value_type, 3>(trianglePoints));
    }
    MeshTriangleAdder<HalfedgeMesh_t>::set_opposite_halfedges(*heMesh);
    return heMesh;
  }

  Derived& set_transformation(const linal::hmat<value_type>& transformation)
  {
    m_transformation = transformation;
    return *static_cast<Derived*>(this);
  }
};

} // namespace Geometry
#endif // GEOMETRY_MESHBUILDERBASE_HPP
