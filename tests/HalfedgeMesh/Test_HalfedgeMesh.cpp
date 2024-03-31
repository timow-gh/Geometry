#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <linal/vec.hpp>

using namespace Geometry;

TEST(HalfedgeMesh, IsDefaultConstructible)
{
  EXPECT_TRUE(std::is_default_constructible_v<HalfedgeMesh<double>>);
}

TEST(HalfedgeMesh, Not_CopyConstructible)
{
  EXPECT_FALSE(std::is_copy_constructible_v<HalfedgeMesh<double>>);
}

TEST(HalfedgeMesh, Not_CopyAssignable)
{
  EXPECT_FALSE(std::is_copy_assignable_v<HalfedgeMesh<double>>);
}

TEST(HalfedgeMesh, IsMoveConstructible)
{
  EXPECT_TRUE(std::is_move_constructible_v<HalfedgeMesh<double>>);
}

TEST(HalfedgeMesh, IsMoveAssignable)
{
  EXPECT_TRUE(std::is_move_assignable_v<HalfedgeMesh<double>>);
}

TEST(HalfedgeMesh, vertexPoints)
{
  HalfedgeMesh<float> heMesh{};
  [[maybe_unused]] std::vector<linal::float3>& vertexPoints = heMesh.getMeshPoints().getPoints();

  const HalfedgeMesh<float>& constMesh = heMesh;
  [[maybe_unused]] const std::vector<linal::float3>& constVertexPoints = constMesh.getMeshPoints().getPoints();
}
