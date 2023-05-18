#ifndef GEOMETRY_MESHBUILDERBASE_HPP
#define GEOMETRY_MESHBUILDERBASE_HPP

#include <Core/Types/TVector.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex, typename Derived>
class MeshBuilderBase {
protected:
  LinAl::HMatrix<TFloat> m_transformation = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> buildTriangleHeMesh(const LinAl::Vec3Vector<TFloat>& points,
                                                                    const Core::TVector<TIndex>& triangleIndices) const
  {
    auto heMesh = std::make_unique<HalfedgeMesh<TFloat, TIndex>>();
    MeshTriangleAdder<TFloat, TIndex> meshTriangleAdder{*heMesh};
    std::size_t size = triangleIndices.size();
    for (std::size_t i{2}; i < size; i += 3)
    {
      LinAl::VecArray<TFloat, 3, 3> trianglePoints;
      for (std::size_t j{0}; j < 3; ++j)
      {
        const std::size_t idx = triangleIndices[i - j];
        const LinAl::Vec3<TFloat>& point = points[idx];
        LinAl::HVec<TFloat> tPoint = m_transformation * LinAl::HVec<TFloat>{point[0], point[1], point[2], 1.0};
        trianglePoints[j] = LinAl::Vec3<TFloat>{tPoint[0], tPoint[1], tPoint[2]};
      }
      meshTriangleAdder(Triangle<TFloat, 3>(trianglePoints));
    }
    return heMesh;
  }

  Derived& setTransformation(const LinAl::HMatrix<TFloat>& transformation)
  {
    m_transformation = transformation;
    return *static_cast<Derived*>(this);
  }
};

} // namespace Geometry
#endif // GEOMETRY_MESHBUILDERBASE_HPP
