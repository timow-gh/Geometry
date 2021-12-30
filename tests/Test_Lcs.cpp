#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Geometry/Cylinder.hpp"
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(Lcs, DefaultConstructor)
{
    auto lcs = Lcs<double_t>();
    EXPECT_EQ(lcs.getOrigin(), LinAl::ZERO_VEC3D);
    EXPECT_EQ(lcs.getXVector(), LinAl::X_VEC3D);
    EXPECT_EQ(lcs.getYVector(), LinAl::Y_VEC3D);
    EXPECT_EQ(lcs.getZVector(), LinAl::Z_VEC3D);
}

TEST(Lcs, Equality)
{
    auto lcsA = Lcs<double_t>();
    auto lcsB = Lcs<double_t>();
    EXPECT_EQ(lcsA, lcsB);
}

TEST(Lcs, Origin_x_y_Constructor)
{
    auto lcsA = Lcs<double_t>();
    auto lcsB = Lcs<double_t>(LinAl::ZERO_VEC3D * 2,
                              LinAl::X_VEC3D,
                              LinAl::Y_VEC3D * 3);
    EXPECT_EQ(lcsA, lcsB);
}

TEST(Lcs, Origin_x_y_z_Constructor)
{
    auto lcsA = Lcs<double_t>();
    auto lcsB = Lcs<double_t>(LinAl::ZERO_VEC3D,
                              LinAl::X_VEC3D * 2,
                              LinAl::Y_VEC3D,
                              LinAl::Z_VEC3D * 6);
    EXPECT_EQ(lcsA, lcsB);
}

TEST(Lcs, Inequality)
{
    auto lcsA = Lcs<double_t>();
    auto lcsB =
        Lcs<double_t>(LinAl::ZERO_VEC3D, LinAl::Y_VEC3D, LinAl::X_VEC3D);
    EXPECT_NE(lcsA, lcsB);
}

#pragma clang diagnostic pop