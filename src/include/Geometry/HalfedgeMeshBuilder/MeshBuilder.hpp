#ifndef MESHLER_MESHBUILDER_HPP
#define MESHLER_MESHBUILDER_HPP

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp"
#include "Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp"
#include "Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshBuilderConfig.hpp"
#include "Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <functional>
#include <memory>

namespace Geometry
{

template <typename T>
class Sphere;

template <typename T>
class Cone;

template <typename T>
class Cylinder;

template <typename T>
class Cuboid;

template <typename TFloat, typename TIndex>
class MeshBuilder {
  MeshBuilderConfig m_config;

public:
  using value_type = TFloat;
  using index_type = TIndex;

  using HalfedgeMesh_t = typename MeshTraits<TFloat, TIndex>::HalfedgeMesh_t;

  constexpr MeshBuilder() noexcept = default;
  constexpr explicit MeshBuilder(const MeshBuilderConfig& meshBuilderConfig) noexcept
      : m_config(meshBuilderConfig)
  {
  }

  GEO_NODISCARD auto build(const Sphere<TFloat>& sphere)
  {
    return Geometry::SphereMeshBuilder<value_type, index_type>()
        .set_polar_count(m_config.polarCount)
        .set_azimuth_count(m_config.azimuthCount)
        .set_sphere(sphere)
        .build();
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Cone<TFloat>& cone)
  {
    return Geometry::ConeMeshBuilder<value_type, index_type>().set_azimuth_count(m_config.azimuthCount).set_cone(cone).build();
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Cylinder<TFloat>& cylinder)
  {
    return Geometry::CylinderMeshBuilder<value_type, index_type>().set_azimuth_count(m_config.azimuthCount).set_cylinder(cylinder).build();
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Cuboid<TFloat>& cuboid)
  {
    return Geometry::CuboidMeshBuilder<value_type, index_type>().set_cuboid(cuboid).build();
  }
};

} // namespace Geometry

#endif // MESHLER_MESHBUILDER_HPP
