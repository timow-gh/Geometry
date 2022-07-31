#ifndef MESHLER_MESHBUILDER_HPP
#define MESHLER_MESHBUILDER_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Fwd/FwdCone.hpp>
#include <Geometry/Fwd/FwdCuboid.hpp>
#include <Geometry/Fwd/FwdCylinder.hpp>
#include <Geometry/Fwd/FwdSphere.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderConfig.hpp>
#include <functional>
#include <memory>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
struct HalfedgeMesh;

class MeshBuilder {
    MeshBuilderConfig m_config;

  public:
    explicit MeshBuilder(const MeshBuilderConfig& meshBuilderConfig);

    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> build(const Geometry::Sphere<double_t>& sphere);
    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> build(const Geometry::Cone<double_t>& cone);
    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> build(const Geometry::Cylinder<double_t>& cylinder);
    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<double_t, std::size_t>> build(const Geometry::Cuboid<double_t>& cuboid);
};
} // namespace Geometry

#endif // MESHLER_MESHBUILDER_HPP
