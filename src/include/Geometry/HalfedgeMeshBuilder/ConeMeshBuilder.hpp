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
  std::size_t m_azimuthCount{20};

public:
  ConeMeshBuilder() = default;

  ConeMeshBuilder& set_cone(const Cone<TFloat>& cone)
  {
    m_cone = cone;
    return *this;
  }

  ConeMeshBuilder& set_azimuth_count(std::size_t azimuthCount)
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
    GEO_ASSERT(linal::is_equal(linal::length(dir), TFloat{1.0}));

    linal::hmat<TFloat> hTrafo;
    linal::rot_align(hTrafo, linal::hvec<TFloat>{0.0, 0.0, 1.0, 1.0}, linal::hvec<TFloat>{dir[0], dir[1], dir[2], 1.0});
    hTrafo.set_translation(coneSeg.get_source());
    MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::set_transformation(hTrafo);

    auto conePoints = calc_cone_points(*m_cone);
    auto coneTriangleIndices = calc_cone_triangle_indices(conePoints);
    return MeshBuilderBase<TFloat, TIndex, ConeMeshBuilder<TFloat, TIndex>>::build_triangle_halfedge_mesh(conePoints, coneTriangleIndices);
  }

private:
  std::vector<linal::vec3<TFloat>> calc_cone_points(const Geometry::Cone<TFloat>& cone) const
  {
    std::vector<linal::vec3<TFloat>> points;

    points.push_back(linal::vec3<TFloat>{0, 0, 0});
    points.push_back(linal::vec3<TFloat>{0, 0, cone.get_segment().length()});

    discretize_circle(points, cone.get_radius(), m_azimuthCount);

    return points;
  }

  std::vector<TIndex> calc_cone_triangle_indices([[maybe_unused]] const std::vector<linal::vec3<TFloat>>& conePoints) const
  {
    TIndex circlePointsStartIdx{2};
    TIndex circlePointsEndIdx = static_cast<TIndex>(conePoints.size() - 1);

    std::vector<TIndex> indices;
    TIndex midPointIdx{0};
    // Bottom circle triangles, face normals point in negative z dir.
    for (TIndex i{circlePointsStartIdx}; i <= circlePointsEndIdx; ++i)
    {
      indices.push_back(midPointIdx);
      indices.push_back(i);
      indices.push_back(i == circlePointsEndIdx ? circlePointsStartIdx : i + 1);
    }

    // Mantle triangles, face normals point outwards
    TIndex peakIdx{1};
    for (TIndex i{circlePointsStartIdx}; i <= circlePointsEndIdx; ++i)
    {
      indices.push_back(peakIdx);
      indices.push_back(i == circlePointsEndIdx ? circlePointsStartIdx : i + 1);
      indices.push_back(i);
    }

    return indices;
  }
};

} // namespace Geometry

#endif // GEOMETRY_CONEMESHBUILDER_HPP
