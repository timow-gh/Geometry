#ifndef GEOMETRY_CONEMESHBUILDER_HPP
#define GEOMETRY_CONEMESHBUILDER_HPP

#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/DiscretizeCircle.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/HMat.hpp>
#include <linal/HMatRotation.hpp>
#include <linal/HMatTranslation.hpp>
#include <linal/utils/Constants.hpp>
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

    linal::hcoord::hmatd hTrafo = linal::hcoord::rot_align(linal::hcoord::Z_HVECD, linal::hcoord::vec3_to_hvec(coneSeg.direction()));
    linal::hcoord::set_translation(hTrafo, coneSeg.getSource());
    MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::setTransformation(hTrafo);

    auto conePoints = calcConePoints(*m_cone);
    auto coneTriangleIndices = calcConeTriangleIndices(conePoints);
    return MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::buildTriangleHeMesh(conePoints, coneTriangleIndices);
  }

private:
  linal::Vec3Vector<TFloat> calcConePoints(const Geometry::Cone<TFloat>& cone) const
  {
    linal::Vec3Vector<TFloat> points;
    discretizeCircle(points, cone.getRadius(), m_azimuthCount);

    points.push_back(linal::Vec3<TFloat>{0, 0, 0});
    points.push_back(linal::Vec3<TFloat>{0, 0, cone.get_segment().length()});

    return points;
  }
  std::vector<TIndex> calcConeTriangleIndices(const linal::Vec3Vector<TFloat>& conePoints) const
  {
    std::vector<TIndex> indices;
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
