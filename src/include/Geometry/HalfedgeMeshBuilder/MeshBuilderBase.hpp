#ifndef GEOMETRY_MESHBUILDERBASE_HPP
#define GEOMETRY_MESHBUILDERBASE_HPP

#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp>
#include <linal/hmat.hpp>
#include <linal/hvec.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <vector>

namespace Geometry
{

template <typename TFloat, typename TIndex, typename Derived>
class MeshBuilderBase {
protected:
  linal::hcoord::hmat<TFloat> m_transformation = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> buildTriangleHeMesh(const linal::vec3vector<TFloat>& points,
                                                                    const std::vector<TIndex>& triangleIndices) const
  {
    auto heMesh = std::make_unique<HalfedgeMesh<TFloat, TIndex>>();
    MeshTriangleAdder<TFloat, TIndex> meshTriangleAdder{*heMesh};
    std::size_t size = triangleIndices.size();
    for (std::size_t i{2}; i < size; i += 3)
    {
      linal::vecArray<TFloat, 3, 3> trianglePoints;
      for (std::size_t j{0}; j < 3; ++j)
      {
        const std::size_t idx = triangleIndices[i - j];
        const linal::vec3<TFloat>& point = points[idx];
        linal::hcoord::hvec<TFloat> tPoint = m_transformation * linal::hcoord::hvec<TFloat>{point[0], point[1], point[2], 1.0};
        trianglePoints[j] = linal::vec3<TFloat>{tPoint[0], tPoint[1], tPoint[2]};
      }
      meshTriangleAdder(Triangle<TFloat, 3>(trianglePoints));
    }
    return heMesh;
  }

  Derived& setTransformation(const linal::hcoord::hmat<TFloat>& transformation)
  {
    m_transformation = transformation;
    return *static_cast<Derived*>(this);
  }
};

} // namespace Geometry
#endif // GEOMETRY_MESHBUILDERBASE_HPP
