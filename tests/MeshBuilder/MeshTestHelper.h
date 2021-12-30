#ifndef GEOMETRY_MESHTESTHELPER_H
#define GEOMETRY_MESHTESTHELPER_H

#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <gtest/gtest.h>

namespace MeshTestHelper
{
testing::AssertionResult isValidIndex(std::size_t index);

// TODO Mesh validation
// isValidVertex();
//
// isValidHalfege();
//
// isValidFacet();
} // namespace MeshTestHelper

#endif // GEOMETRY_MESHTESTHELPER_H
