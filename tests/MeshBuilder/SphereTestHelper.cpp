#include "SphereTestHelper.h"
#include <Geometry/Distance/DistanceSphere.hpp>
#include <linal/utils/Eps.hpp>

namespace MeshTestHelper
{

testing::AssertionResult isPointOnSphere(const Geometry::Sphere<double>& sphere, const linal::Vec3d& vec)
{
  constexpr double DIST_ALLOWED = linal::EPS_D;
  double dist = Geometry::distance(vec, sphere);
  if (linal::isLessEq(dist, DIST_ALLOWED))
    return testing::AssertionSuccess();
  else
    return testing::AssertionFailure() << "Dist: " << dist << ", DistAllowed: " << DIST_ALLOWED;
}
} // namespace MeshTestHelper
