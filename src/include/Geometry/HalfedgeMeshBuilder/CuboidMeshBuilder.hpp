#ifndef GEOMETRY_CUBOIDMESHBUILDER_HPP
#define GEOMETRY_CUBOIDMESHBUILDER_HPP

#include <Core/Types/TArray.hpp>
#include <Core/Utils/Assert.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp>
#include <Geometry/Triangle.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <algorithm>
#include <optional>

namespace Geometry
{

template <typename TFloatType, typename TIndexType = std::size_t>
class CuboidMeshBuilder {
    std::optional<Cuboid<TFloatType>> m_cube;

  public:
    CuboidMeshBuilder() = default;
    CuboidMeshBuilder& setCuboid(const Cuboid<TFloatType>& cube)
    {
        m_cube = cube;
        return *this;
    }

    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<TFloatType, TIndexType>> build()
    {
        CORE_PRECONDITION_DEBUG_ASSERT(m_cube, "Missing cuboid");

        if (!m_cube)
            return nullptr;

        Core::TArray<Triangle<TFloatType, 3>, 12> triangles = calcCuboidTriangles();
        auto heMesh = std::make_unique<HalfedgeMesh<TFloatType, TIndexType>>();
        std::for_each(triangles.cbegin(), triangles.cend(), MeshTriangleAdder<TFloatType, TIndexType>(*heMesh));
        return heMesh;
    }

  private:
    Core::TArray<Triangle<TFloatType, 3>, 12> calcCuboidTriangles()
    {
        auto sides = m_cube->getSideVectors();
        LinAl::Vec3d defaultOrigin{0};

        const LinAl::Vec3<TFloatType>& x = sides[0];
        const LinAl::Vec3<TFloatType>& y = sides[1];
        const LinAl::Vec3<TFloatType>& z = sides[2];

        Core::TArray<Triangle<TFloatType, 3>, 12> triangles;

        LinAl::Vec3d diag = y + z;
        calcCuboidFaceTriangles(triangles, {defaultOrigin, z, diag, y}, m_cube->getOrigin(), x, 0);

        diag = x + z;
        calcCuboidFaceTriangles(triangles, {defaultOrigin, x, diag, z}, m_cube->getOrigin(), y, 4);

        diag = x + y;
        calcCuboidFaceTriangles(triangles, {defaultOrigin, y, diag, x}, m_cube->getOrigin(), z, 8);

        return triangles;
    }

    void calcCuboidFaceTriangles(Core::TArray<Triangle<TFloatType, 3>, 12>& triangles,
                                 LinAl::Vec3Array<TFloatType, 4> vectors,
                                 const LinAl::Vec3<TFloatType>& origin,
                                 const LinAl::Vec3<TFloatType>& translationVec,
                                 std::size_t insertIndex)
    {
        for (auto& vec: vectors)
            vec = vec + origin;
        triangles[insertIndex++] = Triangle<TFloatType, 3>(vectors[0], vectors[1], vectors[2]);
        triangles[insertIndex++] = Triangle<TFloatType, 3>(vectors[2], vectors[3], vectors[0]);

        for (auto& vec: vectors)
            vec = vec + translationVec;
        triangles[insertIndex++] = Triangle<TFloatType, 3>(vectors[2], vectors[1], vectors[0]);
        triangles[insertIndex] = Triangle<TFloatType, 3>(vectors[0], vectors[3], vectors[2]);
    }
};

} // namespace Geometry

#endif // GEOMETRY_CUBOIDMESHBUILDER_HPP
