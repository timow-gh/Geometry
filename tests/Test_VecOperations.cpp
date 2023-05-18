#include <gtest/gtest.h>
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>

TEST(dotProduct, dotZeros)
{
  auto zeros = linal::vec3d(0);
  EXPECT_DOUBLE_EQ(linal::dot(zeros, zeros), 0.0);
}

TEST(dotProduct, dotOnes)
{
  auto ones = linal::vec3d(1);
  EXPECT_DOUBLE_EQ(linal::dot(ones, ones), 3.0);
}

TEST(vectorNorm, zero)
{
  auto zeros = linal::vec3d(0);
  EXPECT_DOUBLE_EQ(linal::norm2(zeros), 0);
}

TEST(vectorNorm, ones)
{
  auto ones = linal::vec3d(1);
  EXPECT_DOUBLE_EQ(linal::norm2(ones), sqrt(3));
}

TEST(vectorProjection, unit)
{
  linal::vec3d vec({1.0, 0.0, 1.0});
  linal::vec3d xVec({5.0, 0.0, 0.0});
  auto projVect = linal::projection(vec, xVec);
  linal::vec3d expected({1.0, 0.0, 0.0});
  EXPECT_EQ(projVect, expected);
}

TEST(vectorProjection, oppositeDirection)
{
  linal::vec3d vec({-2.0, 0.0, 1.0});
  auto projVect = linal::projection(vec, linal::X_VEC3D);
  linal::vec3d expected({-2.0, 0.0, 0.0});
  EXPECT_EQ(projVect, expected);
}

TEST(vectorProjection, twoUnits)
{
  linal::vec3d vec({2.0, 0.0, 1.0});
  linal::vec3d xVec({5.0, 0.0, 0.0});
  auto projVect = linal::projection(vec, xVec);
  linal::vec3d expected({2.0, 0.0, 0.0});
  EXPECT_EQ(projVect, expected);
}

TEST(Vector_Rejection, UnitVec)
{
  linal::vec3d vec({1.0, 0.0, 1.0});
  auto rejection = linal::rejection(vec, linal::X_VEC3D);
  linal::vec3d expected({0.0, 0.0, 1.0});
  EXPECT_EQ(rejection, expected);
}

TEST(Vector_Rejection, zeroVec)
{
  linal::vec3d vec({0.0, 0.0, 0.0});
  auto rejection = linal::rejection(vec, linal::X_VEC3D);
  linal::vec3d expected({0.0, 0.0, 0.0});
  EXPECT_EQ(rejection, expected);
}

TEST(Vector_Rejection, rejectionTestA)
{
  linal::vec3d vec({3.0, 0.0, 9.0});
  auto rejection = linal::rejection(vec, linal::X_VEC3D);
  linal::vec3d expected({0.0, 0.0, 9.0});
  EXPECT_EQ(rejection, expected);
}