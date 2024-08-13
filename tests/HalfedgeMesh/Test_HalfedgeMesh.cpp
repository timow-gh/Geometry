#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <linal/vec.hpp>

using namespace Geometry;

using HeMesh = HalfedgeMesh<double>;

TEST(HalfedgeMesh, IsDefaultConstructible)
{
  EXPECT_TRUE(std::is_default_constructible_v<HeMesh>);
}

TEST(HalfedgeMesh, Not_CopyConstructible)
{
  EXPECT_FALSE(std::is_copy_constructible_v<HeMesh>);
}

TEST(HalfedgeMesh, Not_CopyAssignable)
{
  EXPECT_FALSE(std::is_copy_assignable_v<HeMesh>);
}

TEST(HalfedgeMesh, IsMoveConstructible)
{
  EXPECT_TRUE(std::is_move_constructible_v<HeMesh>);
}

TEST(HalfedgeMesh, IsMoveAssignable)
{
  EXPECT_TRUE(std::is_move_assignable_v<HeMesh>);
}

TEST(HalfedgeMesh, vertexPoints)
{
  HalfedgeMesh<float> heMesh{};
  [[maybe_unused]] std::vector<linal::float3>& vertexPoints = heMesh.getMeshPoints().getPoints();

  const HalfedgeMesh<float>& constMesh = heMesh;
  [[maybe_unused]] const std::vector<linal::float3>& constVertexPoints = constMesh.getMeshPoints().getPoints();
}
