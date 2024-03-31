#include "SphereTestHelper.h"
#include <Geometry/Distance/DistanceSphere.hpp>
#include <linal/utils/eps.hpp>

namespace MeshTestHelper
{

testing::AssertionResult isPointOnSphere(const Geometry::Sphere<double>& sphere, const linal::double3& vec)
{
  constexpr double DIST_ALLOWED = linal::eps_f64;
  double dist = Geometry::distance(vec, sphere);
  if (linal::isLessEq(dist, DIST_ALLOWED))
    return testing::AssertionSuccess();
  else
    return testing::AssertionFailure() << "Dist: " << dist << ", DistAllowed: " << DIST_ALLOWED;
}
} // namespace MeshTestHelper
