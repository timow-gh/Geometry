#ifndef GEOMETRY_MESHBUILDERBASE_HPP
#define GEOMETRY_MESHBUILDERBASE_HPP

#include <Core/Types/TVector.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class MeshBuilderBase
{
  protected:
    std::unique_ptr<HalfedgeMesh<T>>
    buildTriangleHeMesh(const LinAl::Vec3Vector<T>& points,
                        const Core::TVector<uint32_t>& triangleIndices)
    {
        auto heMesh = std::make_unique<HalfedgeMesh<T>>();
        std::size_t size = triangleIndices.size();
        for (std::size_t i{2}; i < size; i += 3)
        {
            std::size_t a = triangleIndices[i - 2];
            std::size_t b = triangleIndices[i - 1];
            std::size_t c = triangleIndices[i];

            addTriangle(heMesh.get(),
                        Triangle<T, 3>(points[a], points[b], points[c]));
        }
        return heMesh;
    }
};

} // namespace Geometry
#endif // GEOMETRY_MESHBUILDERBASE_HPP
