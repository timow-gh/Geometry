#ifndef GEOMETRY_SPHERETESTHELPER_H
#define GEOMETRY_SPHERETESTHELPER_H

#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>

namespace MeshTestHelper
{
testing::AssertionResult isPointOnSphere(const Geometry::Sphere<double>& sphere, const linal::double3& vec);
} // namespace MeshTestHelper

#endif // GEOMETRY_SPHERETESTHELPER_H
