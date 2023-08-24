#ifndef MESHLER_MESHBUILDER_HPP
#define MESHLER_MESHBUILDER_HPP

#include "Geometry/GeometryExport.hpp"
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

template <typename TFloat, typename TIndex>
class HalfedgeMesh;

class GEOMETRY_EXPORT MeshBuilder {
  MeshBuilderConfig m_config;

public:
  constexpr explicit MeshBuilder(const MeshBuilderConfig& meshBuilderConfig);

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Sphere<double>& sphere);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Cone<double>& cone);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Cylinder<double>& cylinder);
  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<double, std::size_t>> build(const Geometry::Cuboid<double>& cuboid);
};
} // namespace Geometry

#endif // MESHLER_MESHBUILDER_HPP
