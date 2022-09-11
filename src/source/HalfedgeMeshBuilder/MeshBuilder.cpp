#include <Geometry/Cone.hpp>
#include <Geometry/Cuboid.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilder.hpp>
#include <Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp>
#include <Geometry/Sphere.hpp>

namespace Geometry
{
MeshBuilder::MeshBuilder(const MeshBuilderConfig& meshBuilderConfig)
    : m_config(meshBuilderConfig)
{
}
std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> MeshBuilder::build(const Sphere<double_t>& sphere)
{
  return Geometry::SphereMeshBuilder<double_t>()
      .setPolarCount(m_config.polarCount)
      .setAzimuthCount(m_config.azimuthCount)
      .setSphere(sphere)
      .build();
}
std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> MeshBuilder::build(const Cone<double_t>& cone)
{
  return Geometry::ConeMeshBuilder<double_t>().setAzimuthCount(m_config.azimuthCount).setCone(cone).build();
}
std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> MeshBuilder::build(const Cylinder<double_t>& cylinder)
{
  return Geometry::CylinderMeshBuilder<double_t>().setAzimuthCount(m_config.azimuthCount).setCylinder(cylinder).build();
}
std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> MeshBuilder::build(const Cuboid<double_t>& cuboid)
{
  return Geometry::CuboidMeshBuilder<double_t>().setCuboid(cuboid).build();
}
} // namespace Geometry