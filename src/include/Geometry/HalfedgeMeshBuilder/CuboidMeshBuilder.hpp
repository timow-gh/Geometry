#ifndef GEOMETRY_CUBOIDMESHBUILDER_HPP
#define GEOMETRY_CUBOIDMESHBUILDER_HPP

#include "Core/Types/TArray.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/Triangle.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <algorithm>

namespace Geometry
{

template <typename T>
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
    triangles[insertIndex] = Triangle<T, 3>(vectors[0], vectors[3], vectors[2]);
}

template <typename T>
Core::TArray<Triangle<T, 3>, 12> calcCuboidTriangles(const Cuboid<T>& cube)
{
    auto sides = cube.sideVectors();
    LinAl::Vec3d defaultOrigin(0);

    const LinAl::Vec3<T>& x = sides[0];
    const LinAl::Vec3<T>& y = sides[1];
    const LinAl::Vec3<T>& z = sides[2];

    Core::TArray<Triangle<T, 3>, 12> triangles;

    LinAl::Vec3d diag = y + z;
    calcCuboidFaceTriangles(triangles,
                            {defaultOrigin, z, diag, y},
                            cube.origin(),
                            x,
                            0);

    diag = x + z;
    calcCuboidFaceTriangles(triangles,
                            {defaultOrigin, x, diag, z},
                            cube.origin(),
                            y,
                            4);

    diag = x + y;
    calcCuboidFaceTriangles(triangles,
                            {defaultOrigin, y, diag, x},
                            cube.origin(),
                            z,
                            8);

    return triangles;
}

} // namespace Geometry

#endif // GEOMETRY_CUBOIDMESHBUILDER_HPP
