#ifndef GEOMETRY_CYLINDERMESHBUILDER_HPP
#define GEOMETRY_CYLINDERMESHBUILDER_HPP

#include "Geometry/Cylinder.hpp"
#include "Geometry/HalfedgeMeshBuilder/DiscretizeCircle.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp"
#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>
#include <optional>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class CylinderMeshBuilder : public MeshBuilderBase<TFloat, TIndex, CylinderMeshBuilder<TFloat, TIndex>> {
  std::optional<Cylinder<TFloat>> m_cylinder;
  TIndex m_azimuthCount{20};

public:
  CylinderMeshBuilder() = default;

  CylinderMeshBuilder& set_cylinder(const Cylinder<TFloat>& clyinder)
  {
    m_cylinder = clyinder;
    return *this;
  }

  CylinderMeshBuilder& set_azimuth_count(TIndex azimuthCount)
  {
    m_azimuthCount = azimuthCount;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build()
  {
    GEO_ASSERT(m_cylinder);
    if (!m_cylinder)
    {
      return nullptr;
    }

    const auto cylinderSeg = m_cylinder->get_segment();

    linal::hmat<TFloat> hTrafo;
    linal::rot_align(hTrafo, linal::hvec<TFloat>{0, 0, 1, 1}, linal::to_hvec(cylinderSeg.direction()));
    hTrafo.set_translation(cylinderSeg.get_source());
    MeshBuilderBase<TFloat, TIndex, CylinderMeshBuilder<TFloat, TIndex>>::set_transformation(hTrafo);

    std::vector<linal::vec3<TFloat>> cylPoints;
    std::vector<TIndex> cylinderTriangleIndices;
    calc_cylinder_triangles(cylPoints, cylinderTriangleIndices);

    return MeshBuilderBase<TFloat, TIndex, CylinderMeshBuilder<TFloat, TIndex>>::build_triangle_halfedge_mesh(cylPoints,
                                                                                                              cylinderTriangleIndices);
  }

private:
  void calc_cylinder_triangles(std::vector<linal::vec3<TFloat>>& points, std::vector<TIndex>& indices) const
  {
    calc_circle_triangles(linal::vec3<TFloat>{0, 0, 0},
                          m_cylinder->get_radius(),
                          m_azimuthCount,
                          CircleDiscretizationDirection::CCW,
                          std::back_inserter(points),
                          TIndex{0},
                          std::back_inserter(indices));

    const Segment3<TFloat>& segment = m_cylinder->get_segment();
    calc_circle_triangles(segment.get_target(),
                          m_cylinder->get_radius(),
                          m_azimuthCount,
                          CircleDiscretizationDirection::CW,
                          std::back_inserter(points),
                          to_idx<TIndex>(points.size()),
                          std::back_inserter(indices));

    // Cylinder mantle triangles
    // Points are ordered as follows:
    // 0: bottom circle center
    // 1, ..., m_azimuthCount: bottom circle points
    // m_azimuthCount + 1: top circle center
    // m_azimuthCount + 2, ..., 2 * m_azimuthCount + 1: top circle points
    // The bottom circle and the top circle start at the same azimuth angle,
    // but the points are ordered in opposite directions.
    // The topIdx corresponding to the bottomIdx is thus: topIdx = bottomIdx + m_azimuthCount + 1

    TIndex bottomStartIdx = 1;
    TIndex bottomEndIdx = m_azimuthCount;
    TIndex topStartIdx = bottomEndIdx + 2;
    TIndex topEndIdx = 2 * m_azimuthCount + 1;

    GEO_ASSERT((bottomEndIdx - bottomStartIdx) == m_azimuthCount - 1);
    GEO_ASSERT((topEndIdx - topStartIdx) == m_azimuthCount - 1);
    GEO_ASSERT((bottomEndIdx - bottomStartIdx) == (topEndIdx - topStartIdx));

    auto tIdxMinusOne = [topStartIdx, topEndIdx](TIndex idx) -> TIndex { return (idx == topStartIdx) ? topEndIdx : idx - 1; };

    TIndex bIdx{bottomStartIdx};
    TIndex tIdx{topStartIdx};
    while (bIdx < bottomEndIdx)
    {
      indices.push_back(bIdx);
      indices.push_back(tIdx);
      indices.push_back(bIdx + 1);

      indices.push_back(tIdx);
      indices.push_back(tIdxMinusOne(tIdx));
      indices.push_back(bIdx + 1);

      bIdx++;
      tIdx = tIdxMinusOne(tIdx);
    }

    indices.push_back(bottomEndIdx);
    indices.push_back(topStartIdx + 1);
    indices.push_back(bottomStartIdx);

    indices.push_back(topStartIdx + 1);
    indices.push_back(topStartIdx);
    indices.push_back(bottomStartIdx);
  }

  std::vector<linal::vec3<TFloat>> calc_cylinder_points() const
  {
    std::vector<linal::vec3<TFloat>> points;
    points.reserve(2 * m_azimuthCount + 2);

    const Segment3<TFloat>& segment = m_cylinder->get_segment();
    discretize_circle(points, m_cylinder->get_radius(), m_azimuthCount);

    TIndex circlePointsSize = to_idx<TIndex>(points.size());

    //    TIndex bottomStartIdx = 0;
    //    TIndex bottomEndIdx = circlePointsSize - 1;
    TIndex topStartIdx = circlePointsSize;
    TIndex topEndIdx = 2 * m_azimuthCount - 1;

    TFloat segLength = segment.length();
    for (TIndex idx{topStartIdx}; idx <= topEndIdx; ++idx)
    {
      linal::vec3<TFloat> bottomPoint = points[idx];
      linal::vec3<TFloat> topPoint = bottomPoint + linal::vec3<TFloat>{0, 0, segLength};
      points[idx] = topPoint;
    }

    points.push_back(linal::vec3<TFloat>{0, 0, 0});
    points.push_back(linal::vec3<TFloat>{0, 0, segLength});

    return points;
  }

  void calc_circle_buffer_indices(std::vector<TIndex>& indices,
                                  const TIndex midPointIdx,
                                  const TIndex circleStartIdx,
                                  const TIndex circleEndIdx) const
  {
    for (TIndex i{circleStartIdx + 1}; i < circleEndIdx; i++)
    {
      indices.push_back(midPointIdx);
      indices.push_back(i - 1);
      indices.push_back(i);
    }
    indices.push_back(midPointIdx);
    indices.push_back(circleStartIdx);
    indices.push_back(circleEndIdx - 1);
  }

  std::vector<TIndex> calc_cylinder_triangle_indices(const std::vector<linal::vec3<TFloat>>& cylinderPoints) const
  {
    // Bottom circle
    std::vector<TIndex> indices;
    const TIndex cylPointsSize = to_idx<TIndex>(cylinderPoints.size());

    const TIndex bottomMidPointIdx = cylPointsSize - 2;
    const TIndex bottomCircleStartIdx = 0;
    const TIndex bottomCircleEndIdx = 0 + m_azimuthCount;
    calc_circle_buffer_indices(indices, bottomMidPointIdx, bottomCircleStartIdx, bottomCircleEndIdx);

    // Outer surface
    for (TIndex i{1}; i < m_azimuthCount; ++i)
    {
      const TIndex topIdx = i + m_azimuthCount;
      const TIndex topIdxMOne = i + m_azimuthCount - 1;
      indices.push_back(i);
      indices.push_back(topIdx);
      indices.push_back(topIdxMOne);

      indices.push_back(i);
      indices.push_back(topIdxMOne);
      indices.push_back(i - 1);
    }
    const TIndex topIdxStart = m_azimuthCount;
    const TIndex topIdxEnd = 2 * m_azimuthCount - 1;
    indices.push_back(topIdxEnd);
    indices.push_back(topIdxStart - 1);
    indices.push_back(0);
    indices.push_back(topIdxEnd);
    indices.push_back(0);
    indices.push_back(topIdxStart);

    // Top circle
    const TIndex topMidPointIdx = cylPointsSize - 1;
    const TIndex topCircleStartIdx = m_azimuthCount;
    const TIndex topCircleEndIdx = 2 * m_azimuthCount;
    calc_circle_buffer_indices(indices, topMidPointIdx, topCircleStartIdx, topCircleEndIdx);

    return indices;
  }
};

} // namespace Geometry

#endif // GEOMETRY_CYLINDERMESHBUILDER_HPP
