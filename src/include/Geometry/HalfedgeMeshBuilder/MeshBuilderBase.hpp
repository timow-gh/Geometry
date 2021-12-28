#ifndef GEOMETRY_MESHBUILDERBASE_HPP
#define GEOMETRY_MESHBUILDERBASE_HPP

#include <Core/Types/TVector.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T, typename Derived>
class MeshBuilderBase {
  protected:
    LinAl::HMatrix<double_t> m_transformation = {{1, 0, 0, 0},
                                                 {0, 1, 0, 0},
                                                 {0, 0, 1, 0},
                                                 {0, 0, 0, 1}};

    std::unique_ptr<HalfedgeMesh<T>>
    buildTriangleHeMesh(const LinAl::Vec3Vector<T>& points,
                        const Core::TVector<uint32_t>& triangleIndices) const
    {
        auto heMesh = std::make_unique<HalfedgeMesh<T>>();
        std::size_t size = triangleIndices.size();
        for (std::size_t i{2}; i < size; i += 3)
        {
            std::size_t a = triangleIndices[i - 2];
            std::size_t b = triangleIndices[i - 1];
            std::size_t c = triangleIndices[i];

            LinAl::VecArray<T, 3, 3> trianglePoints;
            for (std::size_t j{0}; j < 3; ++j)
            {
                const std::size_t idx = triangleIndices[i - j];
                const LinAl::Vec3<T>& point = points[idx];
                LinAl::HVec<T> tPoint =
                    m_transformation * LinAl::HVec<T>{point[0], point[1], point[2], 1.0};
                trianglePoints[j] = LinAl::Vec3<T>{tPoint[0], tPoint[1], tPoint[2]};
            }

            addTriangle(heMesh.get(), Triangle<T, 3>(trianglePoints));
        }
        return heMesh;
    }

    Derived& setTransformation(const LinAl::HMatrix<T>& transformation)
    {
        m_transformation = transformation;
        return *static_cast<Derived*>(this);
    }
};

} // namespace Geometry
#endif // GEOMETRY_MESHBUILDERBASE_HPP
