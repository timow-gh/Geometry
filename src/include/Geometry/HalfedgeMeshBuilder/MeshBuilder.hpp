#ifndef MESHLER_MESHBUILDER_HPP
#define MESHLER_MESHBUILDER_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Fwd/FwdCone.hpp>
#include <Geometry/Fwd/FwdCuboid.hpp>
#include <Geometry/Fwd/FwdCylinder.hpp>
#include <Geometry/Fwd/FwdSphere.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderConfig.hpp>
#include <functional>
#include <memory>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class HalfedgeMesh;

class MeshBuilder {
  MeshBuilderConfig m_config;

public:
  explicit MeshBuilder(const MeshBuilderConfig& meshBuilderConfig);

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Sphere<double>& sphere);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Cone<double>& cone);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Cylinder<double>& cylinder);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Cuboid<double>& cuboid);
};
} // namespace Geometry

#endif // MESHLER_MESHBUILDER_HPP
