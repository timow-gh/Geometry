#include "SphereTestHelper.h"
#include <Geometry/Distance/DistanceSphere.hpp>
#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <linal/utils/eps.hpp>

namespace MeshTestHelper
{

testing::AssertionResult is_point_on_sphere(const Geometry::Sphere<double>& sphere, const linal::double3& vec)
{
  constexpr double distAllowed = linal::eps_f64;
  double dist = Geometry::distance(vec, sphere);
  if (linal::isLessEq(dist, distAllowed)) {
    return testing::AssertionSuccess();
  }

  return testing::AssertionFailure() << "Dist: " << dist << ", DistAllowed: " << distAllowed;
}
} // namespace MeshTestHelper
