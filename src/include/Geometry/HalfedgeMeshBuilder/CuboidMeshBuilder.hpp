#ifndef GEOMETRY_CUBOIDMESHBUILDER_HPP
#define GEOMETRY_CUBOIDMESHBUILDER_HPP

#include "Core/Types/TArray.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/Triangle.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <algorithm>
#include <optional>

namespace Geometry
{

template <typename T>
class CuboidMeshBuilder
{
    std::optional<Cuboid<T>> m_cube;

  public:
    CuboidMeshBuilder& setCuboid(const Cuboid<T>& cube)
    {
        m_cube = cube;
        return *this;
    }

    [[nodiscard]] std::unique_ptr<HalfedgeMesh<T>> build()
    {
        if (!m_cube)
            return nullptr;

        Core::TArray<Triangle<T, 3>, 12> triangles = calcCuboidTriangles();
        auto heMesh = std::make_unique<HalfedgeMesh<T>>(xg::newGuid());
        for (const auto& triangle: triangles)
            addTriangle<T>(heMesh.get(), triangle);
        return std::move(heMesh);
    }

  private:
    Core::TArray<Triangle<T, 3>, 12> calcCuboidTriangles()
    {
        auto sides = m_cube->sideVectors();
        LinAl::Vec3d defaultOrigin(0);

        const LinAl::Vec3<T>& x = sides[0];
        const LinAl::Vec3<T>& y = sides[1];
        const LinAl::Vec3<T>& z = sides[2];

        Core::TArray<Triangle<T, 3>, 12> triangles;

        LinAl::Vec3d diag = y + z;
        calcCuboidFaceTriangles(triangles,
                                {defaultOrigin, z, diag, y},
                                m_cube->origin(),
                                x,
                                0);

        diag = x + z;
        calcCuboidFaceTriangles(triangles,
                                {defaultOrigin, x, diag, z},
                                m_cube->origin(),
                                y,
                                4);

        diag = x + y;
        calcCuboidFaceTriangles(triangles,
                                {defaultOrigin, y, diag, x},
                                m_cube->origin(),
                                z,
                                8);

        return triangles;
    }

    void calcCuboidFaceTriangles(Core::TArray<Triangle<T, 3>, 12>& triangles,
                                 LinAl::Vec3Array<T, 4> vectors,
                                 const LinAl::Vec3<T>& origin,
                                 const LinAl::Vec3<T>& translationVec,
                                 std::size_t insertIndex)
    {
        for (auto& vec: vectors)
            vec = vec + origin;
        triangles[insertIndex++] =
            Triangle<T, 3>(vectors[0], vectors[1], vectors[2]);
        triangles[insertIndex++] =
            Triangle<T, 3>(vectors[2], vectors[3], vectors[0]);

        for (auto& vec: vectors)
            vec = vec + translationVec;
        triangles[insertIndex++] =
            Triangle<T, 3>(vectors[2], vectors[1], vectors[0]);
        triangles[insertIndex] =
            Triangle<T, 3>(vectors[0], vectors[3], vectors[2]);
    }
};

} // namespace Geometry

#endif // GEOMETRY_CUBOIDMESHBUILDER_HPP
