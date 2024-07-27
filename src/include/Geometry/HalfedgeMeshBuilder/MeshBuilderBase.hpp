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

template <typename TFloat, typename TIndex, typename Derived>
class MeshBuilderBase {
protected:
  linal::hmat<TFloat> m_transformation = linal::hmat<TFloat>::identity();

  std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build_triangle_halfedge_mesh(const std::vector<linal::vec3<TFloat>>& points,
                                                                             const std::vector<TIndex>& triangleIndices) const
  {
    auto heMesh = std::make_unique<HalfedgeMesh<TFloat, TIndex>>();
    MeshTriangleAdder<TFloat, TIndex> meshTriangleAdder{*heMesh};
    std::size_t size = triangleIndices.size();
    for (std::size_t i{2}; i < size; i += 3)
    {
      std::array<linal::vec3<TFloat>, 3> trianglePoints;
      for (std::size_t j{0}; j < 3; ++j)
      {
        const std::size_t idx = triangleIndices[i - j];
        const linal::vec3<TFloat>& point = points[idx];
        linal::hvec<TFloat> tPoint = m_transformation * linal::hvec<TFloat>{point[0], point[1], point[2], 1.0};
        trianglePoints[j] = linal::vec3<TFloat>{tPoint[0], tPoint[1], tPoint[2]};
      }
      meshTriangleAdder(Triangle<TFloat, 3>(trianglePoints));
    }
    MeshTriangleAdder<TFloat, TIndex>::set_opposite_halfedges(*heMesh);
    return heMesh;
  }

  Derived& set_transformation(const linal::hmat<TFloat>& transformation)
  {
    m_transformation = transformation;
    return *static_cast<Derived*>(this);
  }
};

} // namespace Geometry
#endif // GEOMETRY_MESHBUILDERBASE_HPP
