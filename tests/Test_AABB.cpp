#include <Geometry/AABB.hpp>
#include <Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp>
#include <Geometry/Sphere.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class AABBTest2f : public ::testing::Test {
protected:
  AABB2f aabb{{0, 0}, 1.0f};
};

TEST_F(AABBTest2f, aabb2f_constructor)
{
  LinAl::Vec2d expectedOrigin{0, 0};
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
  LinAl::Vec3d expectedOrigin{0, 0, 0};
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
  LinAl::Vec2d expectedOrigin{0, 0};
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
  LinAl::Vec3d expectedOrigin{0, 0, 0};
  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  for (auto value: aabb.getExtends())
    EXPECT_DOUBLE_EQ(value, 1.0);
}

TEST_F(AABBTest3d, fromcubepoints)
{
  LinAl::Vec3dVector points{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
  AABB3d box = makeAABB(points);

  LinAl::Vec3d expectedOrigin{0.5, 0.5, 0.5};
  EXPECT_EQ(box.getOrigin(), expectedOrigin);

  LinAl::Vec3d expectedExtend{0.5, 0.5, 0.5};
  auto extends = box.getExtends();
  for (size_t i = 0; i < 3; ++i)
    EXPECT_DOUBLE_EQ(extends[i], expectedExtend[i]);
}

TEST_F(AABBTest3d, makeaabb_empty)
{
  LinAl::Vec3dVector points{};
  AABB3d box = makeAABB(points);

  double negInf = -std::numeric_limits<double>::infinity();
  auto extends = box.getExtends();
  for (auto extend: box.getExtends())
    EXPECT_DOUBLE_EQ(extend, negInf);
}

TEST_F(AABBTest3f, from_sphere_at_origin)
{
  Geometry::Sphere<float> sphere{{0, 0, 0}, 1.0f};
  std::unique_ptr<HalfedgeMesh<float, std::size_t>> sphereMesh =
      Geometry::SphereMeshBuilder<float, std::size_t>().setSphere(sphere).build();
  AABB3f aabb = makeAABB(sphereMesh->getPoints());

  LinAl::Vec3f expectedOrigin{0, 0, 0};
  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  LinAl::Vec3f expectedExtend{1, 1, 1};
  auto extends = aabb.getExtends();
  for (size_t i = 0; i < 3; ++i)
    EXPECT_FLOAT_EQ(extends[i], expectedExtend[i]);
}

TEST_F(AABBTest3f, from_sphere_a)
{
  LinAl::Vec3f expectedOrigin{1, 2, 3};
  Geometry::Sphere<float> sphere{expectedOrigin, 3.2f};
  std::unique_ptr<HalfedgeMesh<float, std::size_t>> sphereMesh =
      Geometry::SphereMeshBuilder<float, std::size_t>().setSphere(sphere).build();
  AABB3f aabb = makeAABB(sphereMesh->getPoints());

  EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

  LinAl::Vec3f expectedExtend{3.2f, 3.2f, 3.2f};
  auto extends = aabb.getExtends();
  for (size_t i = 0; i < 3; ++i)
    EXPECT_FLOAT_EQ(extends[i], expectedExtend[i]);
}