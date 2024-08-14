#ifndef GEOMETRY_CONEMESHBUILDER_HPP
#define GEOMETRY_CONEMESHBUILDER_HPP

#include "Geometry/Cone.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMeshBuilder/DiscretizeCircle.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>
#include <linal/utils/constants.hpp>
#include <optional>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class ConeMeshBuilder : public MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>> {
  std::optional<Cone<TFloat>> m_cone;
  TIndex m_azimuthCount{20};

public:
  ConeMeshBuilder() = default;

  ConeMeshBuilder& set_cone(const Cone<TFloat>& cone)
  {
    m_cone = cone;
    return *this;
  }

  ConeMeshBuilder& set_azimuth_count(TIndex azimuthCount)
  {
    m_azimuthCount = azimuthCount;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build()
  {
    GEO_ASSERT(m_cone);
    if (!m_cone)
    {
      return nullptr;
    }

    const auto coneSeg = m_cone->get_segment();
    GEO_ASSERT(!linal::isZero(coneSeg.length()));

    const auto& dir = coneSeg.direction();
    GEO_ASSERT(linal::isEq(linal::length(dir), TFloat{1.0}));

    linal::hmat<TFloat> hTrafo;
    linal::rot_align(hTrafo, linal::hvec<TFloat>{0.0, 0.0, 1.0, 1.0}, linal::hvec<TFloat>{dir[0], dir[1], dir[2], 1.0});
    hTrafo.set_translation(coneSeg.get_source());
    MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::set_transformation(hTrafo);

    std::vector<linal::vec3<TFloat>> conePoints;
    std::vector<TIndex> coneTriangleIndices;
    calc_cone_triangles(conePoints, coneTriangleIndices);

    return MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::build_triangle_halfedge_mesh(conePoints, coneTriangleIndices);
  }

private:
  void calc_cone_triangles(std::vector<linal::vec3<TFloat>>& conePoints, std::vector<TIndex>& coneTriangleIndices) const
  {
    GEO_ASSERT(m_cone);
    GEO_ASSERT(conePoints.empty());
    GEO_ASSERT(coneTriangleIndices.empty());

    // Bottom circle triangles and indices.
    calc_circle_triangles(
        linal::vec3<TFloat>{0, 0, 0}, // The ConeMeshBuilder assumes the cone to be aligned with the z-axis and placed at the origin.
        m_cone->get_radius(),
        m_azimuthCount,
        CircleDiscretizationDirection::CCW,
        std::back_inserter(conePoints),
        TIndex{0},
        std::back_inserter(coneTriangleIndices));

    // Mantle triangles.
    conePoints.push_back(linal::vec3<TFloat>{0, 0, m_cone->get_segment().length()});
    TIndex circlePointsStartIdx{1}; // The first point is the center of the bottom circle.
    TIndex circlePointsEndIdx = static_cast<TIndex>(conePoints.size() - 2);
    TIndex peakIdx = static_cast<TIndex>(conePoints.size() - 1);
    for (TIndex i{circlePointsStartIdx}; i <= circlePointsEndIdx; ++i)
    {
      coneTriangleIndices.push_back(peakIdx);
      coneTriangleIndices.push_back(i == circlePointsEndIdx ? circlePointsStartIdx : i + 1);
      coneTriangleIndices.push_back(i);
    }
  }
};

} // namespace Geometry

#endif // GEOMETRY_CONEMESHBUILDER_HPP
