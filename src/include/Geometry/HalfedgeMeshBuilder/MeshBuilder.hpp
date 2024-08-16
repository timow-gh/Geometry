#ifndef MESHLER_MESHBUILDER_HPP
#define MESHLER_MESHBUILDER_HPP

#include "Geometry/GeometryExport.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshBuilderConfig.hpp"
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

class GEOMETRY_EXPORT MeshBuilder {
  MeshBuilderConfig m_config;

public:
  using value_type = double;
  using index_type = std::size_t;
  using MeshTraits_t = MeshTraits<value_type, index_type>;
  using HalfedgeMesh_t = HalfedgeMesh<MeshTraits_t>;

  using Sphere_t = Sphere<value_type>;
  using Cone_t = Cone<value_type>;
  using Cylinder_t = Cylinder<value_type>;
  using Cuboid_t = Cuboid<value_type>;

  constexpr explicit MeshBuilder(const MeshBuilderConfig& meshBuilderConfig);

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Sphere_t& sphere);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Cone_t& cone);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Cylinder_t& cylinder);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build(const Cuboid_t& cuboid);
};

} // namespace Geometry

#endif // MESHLER_MESHBUILDER_HPP
