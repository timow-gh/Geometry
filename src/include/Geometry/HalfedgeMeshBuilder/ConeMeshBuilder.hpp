#ifndef GEOMETRY_CONEMESHBUILDER_HPP
#define GEOMETRY_CONEMESHBUILDER_HPP

#include <Core/Math/Constants.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CirclePoints.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <optional>

namespace Geometry
{
template <typename TFloat, typename TIndex>
class ConeMeshBuilder : public MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>> {
  std::optional<Cone<TFloat>> m_cone;
  std::size_t m_azimuthCount{20};

public:
  ConeMeshBuilder() = default;

  ConeMeshBuilder& setCone(const Cone<TFloat>& cone)
  {
    m_cone = cone;
    return *this;
  }

  ConeMeshBuilder& setAzimuthCount(std::size_t azimuthCount)
  {
    m_azimuthCount = azimuthCount;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build()
  {
    if (!m_cone)
      return nullptr;

    const auto coneSeg = m_cone->get_segment();

    LinAl::HMatrixd hTrafo = LinAl::hMatRotationAlign(LinAl::Z_HVECD, LinAl::vec3ToHVec(coneSeg.direction()));
    LinAl::setTranslation(hTrafo, coneSeg.getSource());
    MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::setTransformation(hTrafo);

    auto conePoints = calcConePoints(*m_cone);
    auto coneTriangleIndices = calcConeTriangleIndices(conePoints);
    return MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::buildTriangleHeMesh(conePoints, coneTriangleIndices);
  }

private:
  LinAl::Vec3Vector<TFloat> calcConePoints(const Geometry::Cone<TFloat>& cone) const
  {
    LinAl::Vec3Vector<TFloat> points;
    calcCirclePoints(points, cone.get_radius(), m_azimuthCount);

    points.push_back(LinAl::Vec3<TFloat>{0, 0, 0});
    points.push_back(LinAl::Vec3<TFloat>{0, 0, cone.get_segment().length()});

    return points;
  }
  Core::TVector<TIndex> calcConeTriangleIndices(const LinAl::Vec3Vector<TFloat>& conePoints) const
  {
    Core::TVector<TIndex> indices;
    TIndex size = static_cast<TIndex>(conePoints.size());
    TIndex topIdx = size - 1;
    TIndex bottomIdx = size - 2;
    for (TIndex i{1}; i < size; ++i)
    {
      // slant surface
      indices.push_back(topIdx);
      indices.push_back(i - 1);
      indices.push_back(i);
      // base
      indices.push_back(bottomIdx);
      indices.push_back(i - 1);
      indices.push_back(i);
    }
    // slant surface, connecting triangle
    indices.push_back(topIdx);
    indices.push_back(size - 3);
    indices.push_back(0);
    // base, connecting triangle
    indices.push_back(bottomIdx);
    indices.push_back(size - 3);
    indices.push_back(0);

    return indices;
  }
};
} // namespace Geometry

#endif // GEOMETRY_CONEMESHBUILDER_HPP
