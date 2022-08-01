#ifndef GEOMETRY_SPHERETESTHELPER_H
#define GEOMETRY_SPHERETESTHELPER_H

#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>

namespace MeshTestHelper
{
testing::AssertionResult isPointOnSphere(const Geometry::Sphere<double_t>& sphere, const LinAl::Vec3d& vec);
} // namespace MeshTestHelper

#endif // GEOMETRY_SPHERETESTHELPER_H
