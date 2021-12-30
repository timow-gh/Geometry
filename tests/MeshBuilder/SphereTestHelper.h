#ifndef GEOMETRY_SPHERETESTHELPER_H
#define GEOMETRY_SPHERETESTHELPER_H

#include <gtest/gtest.h>
#include <Geometry/Sphere.hpp>

namespace MeshTestHelper
{
testing::AssertionResult
isPointOnSphere(const Geometry::Sphere<double_t>& sphere,
                const LinAl::Vec3d& vec);
} // namespace MeshTestHelper

#endif // GEOMETRY_SPHERETESTHELPER_H
