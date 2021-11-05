#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Geometry/Cylinder.hpp"
#include "gtest/gtest.h"

using namespace Geometry;
using namespace LinAl;

TEST(Cylinder, Constructor)
{
    constexpr double_t HEIGHT = 5;
    constexpr double_t RADIUS = 1;
    const auto originLcs = Lcs<double_t>();
    Cylinder cylinder{originLcs, HEIGHT, RADIUS};
    EXPECT_EQ(cylinder.getOriginLcs(), originLcs);
    EXPECT_EQ(cylinder.getHeight(), HEIGHT);
    EXPECT_EQ(cylinder.getRadius(), RADIUS);
}

#pragma clang diagnostic pop