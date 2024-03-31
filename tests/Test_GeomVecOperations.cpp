#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

TEST(dotProduct, dotZeros)
{
  auto zeros = linal::double3(0);
  EXPECT_DOUBLE_EQ(linal::dot(zeros, zeros), 0.0);
}

TEST(dotProduct, dotOnes)
{
  auto ones = linal::double3(1);
  EXPECT_DOUBLE_EQ(linal::dot(ones, ones), 3.0);
}

TEST(vectorNorm, zero)
{
  auto zeros = linal::double3(0);
  EXPECT_DOUBLE_EQ(linal::length(zeros), 0);
}

TEST(vectorNorm, ones)
{
  auto ones = linal::double3(1);
  EXPECT_DOUBLE_EQ(linal::length(ones), sqrt(3));
}

TEST(vectorProjection, unit)
{
  linal::double3 vec({1.0, 0.0, 1.0});
  linal::double3 xVec({5.0, 0.0, 0.0});
  auto projVect = linal::projection(vec, xVec);
  linal::double3 expected({1.0, 0.0, 0.0});
  EXPECT_EQ(projVect, expected);
}

TEST(vectorProjection, oppositeDirection)
{
  linal::double3 vec({-2.0, 0.0, 1.0});
  auto projVect = linal::projection(vec, linal::double3X);
  linal::double3 expected({-2.0, 0.0, 0.0});
  EXPECT_EQ(projVect, expected);
}

TEST(vectorProjection, twoUnits)
{
  linal::double3 vec({2.0, 0.0, 1.0});
  linal::double3 xVec({5.0, 0.0, 0.0});
  auto projVect = linal::projection(vec, xVec);
  linal::double3 expected({2.0, 0.0, 0.0});
  EXPECT_EQ(projVect, expected);
}

TEST(Vector_Rejection, UnitVec)
{
  linal::double3 vec({1.0, 0.0, 1.0});
  auto rejection = linal::rejection(vec, linal::double3X);
  linal::double3 expected({0.0, 0.0, 1.0});
  EXPECT_EQ(rejection, expected);
}

TEST(Vector_Rejection, zeroVec)
{
  linal::double3 vec({0.0, 0.0, 0.0});
  auto rejection = linal::rejection(vec, linal::double3X);
  linal::double3 expected({0.0, 0.0, 0.0});
  EXPECT_EQ(rejection, expected);
}

TEST(Vector_Rejection, rejectionTestA)
{
  linal::double3 vec({3.0, 0.0, 9.0});
  auto rejection = linal::rejection(vec, linal::double3X);
  linal::double3 expected({0.0, 0.0, 9.0});
  EXPECT_EQ(rejection, expected);
}