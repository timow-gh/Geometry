#include <Geometry/AABB.hpp>
#include <Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp>
#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>
#include <linal/containers.hpp>
#include <linal/vec2.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

class AABBTest2f : public ::testing::Test {
protected:
  AABB2f aabb{{0, 0}, 1.0f};
};

TEST_F(AABBTest2f, aabb2f_constructor)
{
  linal::vec2d expectedOrigin{0, 0};
  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  for (auto value: aabb.getExtends())
    EXPECT_FLOAT_EQ(value, 1.0f);
}

class AABBTest3f : public ::testing::Test {
protected:
  AABB3f aabb{{0, 0, 0}, 1.0f};
};

TEST_F(AABBTest3f, aabb3f_constructor)
{
  linal::vec3d expectedOrigin{0, 0, 0};
  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  for (auto value: aabb.getExtends())
    EXPECT_FLOAT_EQ(value, 1.0f);
}

class AABBTest2d : public ::testing::Test {
protected:
  AABB2d aabb{{0, 0}, 1.0};
};

TEST_F(AABBTest2d, aabb2f_constructor)
{
  linal::vec2d expectedOrigin{0, 0};
  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  for (auto value: aabb.getExtends())
    EXPECT_DOUBLE_EQ(value, 1.0);
}

class AABBTest3d : public ::testing::Test {
protected:
  AABB3d aabb{{0, 0, 0}, 1.0};
};

TEST_F(AABBTest3d, aabb3f_constructor)
{
  linal::vec3d expectedOrigin{0, 0, 0};
  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  for (auto value: aabb.getExtends())
    EXPECT_DOUBLE_EQ(value, 1.0);
}

TEST_F(AABBTest3d, fromcubepoints)
{
  linal::vec3dvector points{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
  AABB3d box = makeAABB(points);

  linal::vec3d expectedOrigin{0.5, 0.5, 0.5};
  EXPECT_EQ(box.getOrigin(), expectedOrigin);

  linal::vec3d expectedExtend{0.5, 0.5, 0.5};
  auto extends = box.getExtends();
  for (size_t i = 0; i < 3; ++i)
    EXPECT_DOUBLE_EQ(extends[i], expectedExtend[i]);
}

TEST_F(AABBTest3f, from_sphere_at_origin)
{
  Geometry::Sphere<float> sphere{{0, 0, 0}, 1.0f};
  std::unique_ptr<HalfedgeMesh<float, std::size_t>> sphereMesh =
      Geometry::SphereMeshBuilder<float, std::size_t>().setSphere(sphere).build();
  AABB3f box = makeAABB(sphereMesh->getPoints());

  linal::vec3f expectedOrigin{0, 0, 0};
  EXPECT_EQ(box.getOrigin(), expectedOrigin);

  linal::vec3f expectedExtend{1, 1, 1};
  auto extends = box.getExtends();
  for (size_t i = 0; i < 3; ++i)
    EXPECT_FLOAT_EQ(extends[i], expectedExtend[i]);
}

TEST_F(AABBTest3f, from_sphere_a)
{
  linal::vec3f expectedOrigin{1, 2, 3};
  Geometry::Sphere<float> sphere{expectedOrigin, 3.2f};
  std::unique_ptr<HalfedgeMesh<float, std::size_t>> sphereMesh =
      Geometry::SphereMeshBuilder<float, std::size_t>().setSphere(sphere).build();
  AABB3f box = makeAABB(sphereMesh->getPoints());

  EXPECT_EQ(box.getOrigin(), expectedOrigin);

  linal::vec3f expectedExtend{3.2f, 3.2f, 3.2f};
  auto extends = box.getExtends();
  for (size_t i = 0; i < 3; ++i)
    EXPECT_FLOAT_EQ(extends[i], expectedExtend[i]);
}