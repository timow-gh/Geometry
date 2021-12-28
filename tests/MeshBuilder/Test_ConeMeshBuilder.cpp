#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <LinAl/LinearAlgebra.hpp>
#include "gtest/gtest.h"
#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp>

using namespace Geometry;
using namespace LinAl;

TEST(ConeMeshBuilder, createAConeMesh)
{
    auto coneMesh = ConeMeshBuilder<double_t>()
                        .setCone(Cone<double_t>(ZERO_VEC3D, Z_VEC3D, 10.0))
                        .build();
    EXPECT_TRUE(coneMesh);
}

#pragma clang diagnostic pop