#include "Geometry/HalfedgeMesh/HalfedgeMeshIndices.hpp"
#include <gtest/gtest.h>

TEST(MeshIndexBase, DefaultConstructor)
{
    Geometry::MeshIndexBase<std::size_t> idx;
    EXPECT_FALSE(idx.isValid());
}
