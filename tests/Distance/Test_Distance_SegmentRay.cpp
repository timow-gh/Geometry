#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Core/Types/TVector.hpp"
#include "Geometry/Segment.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "gtest/gtest.h"

using namespace Geometry;
using namespace LinAl;

class SegmentRayDistanceTests
    : public ::testing::Test
{
  protected:
    Segment3d m_segment3d{Vec3d{}, LinAl::X_VEC3D};
};

TEST_F(SegmentRayDistanceTests, asdf)
{
    assert(false);
}

#pragma clang diagnostic pop