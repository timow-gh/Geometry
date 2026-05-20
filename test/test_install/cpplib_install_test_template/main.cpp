#include "Geometry/AABB.hpp"

int main()
{
  const Geometry::AABB3d aabb{linal::double3{0.0, 0.0, 0.0}, linal::double3{1.0, 1.0, 1.0}};
  return aabb.is_valid() ? 0 : 1;
}
