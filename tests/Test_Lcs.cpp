#include <Geometry/Lcs.hpp>
#include <gtest/gtest.h>
#include <linal/Vec3.hpp>

using namespace Geometry;

TEST(Lcs, DefaultConstructor)
{
  auto lcs = Lcs<double>();
  EXPECT_EQ(lcs.getOrigin(), linal::ZERO_VEC3D);
  EXPECT_EQ(lcs.getXVector(), linal::X_VEC3D);
  EXPECT_EQ(lcs.getYVector(), linal::Y_VEC3D);
  EXPECT_EQ(lcs.getZVector(), linal::Z_VEC3D);
}

TEST(Lcs, Equality)
{
  auto lcsA = Lcs<double>();
  auto lcsB = Lcs<double>();
  EXPECT_EQ(lcsA, lcsB);
}

TEST(Lcs, Origin_x_y_Constructor)
{
  auto lcsA = Lcs<double>();
  auto lcsB = Lcs<double>(linal::ZERO_VEC3D * 2, linal::X_VEC3D, linal::Y_VEC3D * 3);
  EXPECT_EQ(lcsA, lcsB);
}

TEST(Lcs, Origin_x_y_z_Constructor)
{
  auto lcsA = Lcs<double>();
  auto lcsB = Lcs<double>(linal::ZERO_VEC3D, linal::X_VEC3D * 2, linal::Y_VEC3D, linal::Z_VEC3D * 6);
  EXPECT_EQ(lcsA, lcsB);
}

TEST(Lcs, Inequality)
{
  auto lcsA = Lcs<double>();
  auto lcsB = Lcs<double>(linal::ZERO_VEC3D, linal::Y_VEC3D, linal::X_VEC3D);
  EXPECT_NE(lcsA, lcsB);
}
