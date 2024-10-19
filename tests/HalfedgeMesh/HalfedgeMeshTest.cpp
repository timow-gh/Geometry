#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

using MeshTraits_t = MeshTraits<double, std::size_t>;
using HeMesh = HalfedgeMesh<MeshTraits_t>;

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
  HeMesh heMesh{};
  [[maybe_unused]] std::vector<linal::double3>& vertexPoints = heMesh.getMeshPoints().getPoints();

  const HeMesh& constMesh = heMesh;
  [[maybe_unused]] const std::vector<linal::double3>& constVertexPoints = constMesh.getMeshPoints().getPoints();
}
