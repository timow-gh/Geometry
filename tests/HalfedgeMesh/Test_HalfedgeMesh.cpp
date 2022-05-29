#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <gtest/gtest.h>

using namespace Core;
using namespace LinAl;
using namespace Geometry;

TEST(HalfedgeMesh, IsDefaultConstructible)
{
    EXPECT_TRUE(std::is_default_constructible_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, Not_CopyConstructible)
{
    EXPECT_FALSE(std::is_copy_constructible_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, Not_CopyAssignable)
{
    EXPECT_FALSE(std::is_copy_assignable_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, IsMoveConstructible)
{
    EXPECT_TRUE(std::is_move_constructible_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, IsMoveAssignable)
{
    EXPECT_TRUE(std::is_move_assignable_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, vertexPoints)
{
    HalfedgeMesh<float_t> heMesh{};
    [[maybe_unused]] Vec3Vector<float_t>& vertexPoints = heMesh.meshPoints.getPoints();

    const HalfedgeMesh<float_t>& constMesh = heMesh;
    [[maybe_unused]] const Vec3Vector<float_t>& constVertexPoints = heMesh.meshPoints.getPoints();
}
