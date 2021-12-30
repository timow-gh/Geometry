#include "Geometry/InvalidIndex.hpp"
#include "MeshTestHelper.h"
#include <gtest/gtest.h>

using namespace MeshTestHelper;

TEST(Test_MeshBuilderHelper, isInvalidIndex)
{
    EXPECT_FALSE(isValidIndex(Geometry::INVALID_INDEX));
}
