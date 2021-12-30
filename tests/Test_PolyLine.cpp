#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <Geometry/PolyLine.hpp>
#include <gtest/gtest.h>

using namespace LinAl;
using namespace Geometry;

TEST(PolyLine, Ctor)
{
    Vec3d first = {0, 0, 0};
    Vec3d second = {0, 0, 0};

    Vec3dVector points = {first, second};
    PolyLine3d polyLine = PolyLine3d(points);
    Vec3dVector polyLinePoints = polyLine.getPoints();

    EXPECT_EQ(first, polyLinePoints[0]);
    EXPECT_EQ(second, polyLinePoints[1]);
}



#pragma clang diagnostic pop