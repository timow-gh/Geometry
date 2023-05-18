#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(ConeMeshBuilder, createAConeMesh)
{
    auto coneMesh = ConeMeshBuilder<double, std::size_t>().setCone(Cone<double>(ZERO_VEC3D, Z_VEC3D, 10.0)).build();
    EXPECT_TRUE(coneMesh);
}
