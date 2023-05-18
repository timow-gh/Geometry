#include "SphereTestHelper.h"
#include <Geometry/Distance/DistanceSphere.hpp>

namespace MeshTestHelper
{

testing::AssertionResult isPointOnSphere(const Geometry::Sphere<double>& sphere, const linal::Vec3d& vec)
{
    constexpr double DIST_ALLOWED = Core::EPS_D;
    double dist = Geometry::distance(vec, sphere);
    if (Core::isLessEq(dist, DIST_ALLOWED))
        return testing::AssertionSuccess();
    else
        return testing::AssertionFailure() << "Dist: " << dist << ", DistAllowed: " << DIST_ALLOWED;
}
} // namespace MeshTestHelper
