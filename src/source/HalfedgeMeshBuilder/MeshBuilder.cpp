#include "Geometry/HalfedgeMeshBuilder/MeshBuilder.hpp"
#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp"
#include "Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp"
#include "Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp"
#include "Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp"
#include "Geometry/Sphere.hpp"

namespace Geometry
{

constexpr MeshBuilder::MeshBuilder(const MeshBuilderConfig& meshBuilderConfig)
    : m_config(meshBuilderConfig)
{
}

std::unique_ptr<MeshBuilder::HalfedgeMesh_t> MeshBuilder::build(const Sphere<double>& sphere)
{
  return Geometry::SphereMeshBuilder<value_type, index_type>()
      .set_polar_count(m_config.polarCount)
      .set_azimuth_count(m_config.azimuthCount)
      .set_sphere(sphere)
      .build();
}

std::unique_ptr<MeshBuilder::HalfedgeMesh_t> MeshBuilder::build(const Cone<double>& cone)
{
  return Geometry::ConeMeshBuilder<value_type, index_type>().set_azimuth_count(m_config.azimuthCount).set_cone(cone).build();
}

std::unique_ptr<MeshBuilder::HalfedgeMesh_t> MeshBuilder::build(const Cylinder<double>& cylinder)
{
  return Geometry::CylinderMeshBuilder<value_type, index_type>().set_azimuth_count(m_config.azimuthCount).set_cylinder(cylinder).build();
}

std::unique_ptr<MeshBuilder::HalfedgeMesh_t> MeshBuilder::build(const Cuboid<double>& cuboid)
{
  return Geometry::CuboidMeshBuilder<value_type, index_type>().set_cuboid(cuboid).build();
}

} // namespace Geometry
