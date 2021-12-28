#include <LinAl/LinearAlgebra.hpp>
#include "gtest/gtest.h"

TEST(dotProduct, dotZeros)
{
    auto zeros = LinAl::Vec3d(0);
    EXPECT_DOUBLE_EQ(LinAl::dot(zeros, zeros), 0.0);
}

TEST(dotProduct, dotOnes)
{
    auto ones = LinAl::Vec3d(1);
    EXPECT_DOUBLE_EQ(LinAl::dot(ones, ones), 3.0);
}

TEST(vectorNorm, zero)
{
    auto zeros = LinAl::Vec3d(0);
    EXPECT_DOUBLE_EQ(LinAl::norm2(zeros), 0);
}

TEST(vectorNorm, ones)
{
    auto ones = LinAl::Vec3d(1);
    EXPECT_DOUBLE_EQ(LinAl::norm2(ones), sqrt(3));
}

TEST(vectorProjection, unit)
{
    LinAl::Vec3d vec({1.0, 0.0, 1.0});
    LinAl::Vec3d xVec({5.0, 0.0, 0.0});
    auto projVect = LinAl::projection(vec, xVec);
    LinAl::Vec3d expected({1.0, 0.0, 0.0});
    EXPECT_EQ(projVect, expected);
}

TEST(vectorProjection, oppositeDirection)
{
    LinAl::Vec3d vec({-2.0, 0.0, 1.0});
    auto projVect = LinAl::projection(vec, LinAl::X_VEC3D);
    LinAl::Vec3d expected({-2.0, 0.0, 0.0});
    EXPECT_EQ(projVect, expected);
}

TEST(vectorProjection, twoUnits)
{
    LinAl::Vec3d vec({2.0, 0.0, 1.0});
    LinAl::Vec3d xVec({5.0, 0.0, 0.0});
    auto projVect = LinAl::projection(vec, xVec);
    LinAl::Vec3d expected({2.0, 0.0, 0.0});
    EXPECT_EQ(projVect, expected);
}

TEST(Vector_Rejection, UnitVec)
{
    LinAl::Vec3d vec({1.0, 0.0, 1.0});
    auto rejection = LinAl::rejection(vec, LinAl::X_VEC3D);
    LinAl::Vec3d expected({0.0, 0.0, 1.0});
    EXPECT_EQ(rejection, expected);
}

TEST(Vector_Rejection, zeroVec)
{
    LinAl::Vec3d vec({0.0, 0.0, 0.0});
    auto rejection = LinAl::rejection(vec, LinAl::X_VEC3D);
    LinAl::Vec3d expected({0.0, 0.0, 0.0});
    EXPECT_EQ(rejection, expected);
}

TEST(Vector_Rejection, rejectionTestA)
{
    LinAl::Vec3d vec({3.0, 0.0, 9.0});
    auto rejection = LinAl::rejection(vec, LinAl::X_VEC3D);
    LinAl::Vec3d expected({0.0, 0.0, 9.0});
    EXPECT_EQ(rejection, expected);
}