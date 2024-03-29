#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <gtest/gtest.h>
#include <linal/containers.hpp>
#include <linal/vec3.hpp>

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
  [[maybe_unused]] linal::vec3vector<float>& vertexPoints = heMesh.getMeshPoints().getPoints();

  const HalfedgeMesh<float>& constMesh = heMesh;
  [[maybe_unused]] const linal::vec3vector<float>& constVertexPoints = constMesh.getMeshPoints().getPoints();
}
