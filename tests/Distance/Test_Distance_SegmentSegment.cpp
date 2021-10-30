#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Core/Types/TVector.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "gtest/gtest.h"
#include "Geometry/Segment.hpp"

using namespace Geometry;
using namespace LinAl;

class SegmentSegmentDistanceTests
    : public ::testing::Test
{
  protected:
    Segment3d m_segment3d{Vec3d{}, LinAl::X_VEC3D};
};

TEST_F(SegmentSegmentDistanceTests, asdf)
{
    assert(false);
}

#pragma clang diagnostic pop