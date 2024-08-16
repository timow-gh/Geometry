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
class CylinderMeshBuilder : public MeshBuilderBase<MeshTraits<TFloat, TIndex>, CylinderMeshBuilder<TFloat, TIndex>> {
public:
  using MeshTraits_t = MeshTraits<TFloat, TIndex>;

  using value_type = typename MeshTraits_t::value_type;
  using index_type = typename MeshTraits_t::index_type;

  using HalfedgeMesh_t = typename MeshTraits_t::HalfedgeMesh_t;

  using Cylinder_t = Cylinder<value_type>;

  std::optional<Cylinder_t> m_cylinder;
  index_type m_azimuthCount{20};

public:
  CylinderMeshBuilder() = default;

  CylinderMeshBuilder& set_cylinder(const Cylinder_t& clyinder)
  {
    m_cylinder = clyinder;
    return *this;
  }

  CylinderMeshBuilder& set_azimuth_count(index_type azimuthCount)
  {
    m_azimuthCount = azimuthCount;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build()
  {
    GEO_ASSERT(m_cylinder);
    if (!m_cylinder)
    {
      return nullptr;
    }

    const auto cylinderSeg = m_cylinder->get_segment();

    linal::hmat<value_type> hTrafo;
    linal::rot_align(hTrafo, linal::hvec<value_type>{0, 0, 1, 1}, linal::to_hvec(cylinderSeg.direction()));
    hTrafo.set_translation(cylinderSeg.get_source());
    MeshBuilderBase<MeshTraits_t, CylinderMeshBuilder<value_type, index_type>>::set_transformation(hTrafo);

    std::vector<linal::vec3<value_type>> cylPoints;
    std::vector<index_type> cylinderTriangleIndices;
    calc_cylinder_triangles(cylPoints, cylinderTriangleIndices);

    return MeshBuilderBase<MeshTraits_t, CylinderMeshBuilder<value_type, index_type>>::build_triangle_halfedge_mesh(
        cylPoints,
        cylinderTriangleIndices);
  }

private:
  void calc_cylinder_triangles(std::vector<linal::vec3<value_type>>& points, std::vector<index_type>& indices) const
  {
    calc_circle_triangles(linal::vec3<value_type>{0, 0, 0},
                          m_cylinder->get_radius(),
                          m_azimuthCount,
                          CircleDiscretizationDirection::CCW,
                          std::back_inserter(points),
                          index_type{0},
                          std::back_inserter(indices));

    const Segment3<value_type>& segment = m_cylinder->get_segment();
    calc_circle_triangles(segment.get_target(),
                          m_cylinder->get_radius(),
                          m_azimuthCount,
                          CircleDiscretizationDirection::CW,
                          std::back_inserter(points),
                          to_idx<index_type>(points.size()),
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

    index_type bottomStartIdx = 1;
    index_type bottomEndIdx = m_azimuthCount;
    index_type topStartIdx = bottomEndIdx + 2;
    index_type topEndIdx = 2 * m_azimuthCount + 1;

    GEO_ASSERT((bottomEndIdx - bottomStartIdx) == m_azimuthCount - 1);
    GEO_ASSERT((topEndIdx - topStartIdx) == m_azimuthCount - 1);
    GEO_ASSERT((bottomEndIdx - bottomStartIdx) == (topEndIdx - topStartIdx));

    auto tIdxMinusOne = [topStartIdx, topEndIdx](index_type idx) -> index_type { return (idx == topStartIdx) ? topEndIdx : idx - 1; };

    index_type bIdx{bottomStartIdx};
    index_type tIdx{topStartIdx};
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

  std::vector<linal::vec3<value_type>> calc_cylinder_points() const
  {
    std::vector<linal::vec3<value_type>> points;
    points.reserve(2 * m_azimuthCount + 2);

    const Segment3<value_type>& segment = m_cylinder->get_segment();
    discretize_circle(points, m_cylinder->get_radius(), m_azimuthCount);

    index_type circlePointsSize = to_idx<index_type>(points.size());

    //    index_type bottomStartIdx = 0;
    //    index_type bottomEndIdx = circlePointsSize - 1;
    index_type topStartIdx = circlePointsSize;
    index_type topEndIdx = 2 * m_azimuthCount - 1;

    value_type segLength = segment.length();
    for (index_type idx{topStartIdx}; idx <= topEndIdx; ++idx)
    {
      linal::vec3<value_type> bottomPoint = points[idx];
      linal::vec3<value_type> topPoint = bottomPoint + linal::vec3<value_type>{0, 0, segLength};
      points[idx] = topPoint;
    }

    points.push_back(linal::vec3<value_type>{0, 0, 0});
    points.push_back(linal::vec3<value_type>{0, 0, segLength});

    return points;
  }

  void calc_circle_buffer_indices(std::vector<index_type>& indices,
                                  const index_type midPointIdx,
                                  const index_type circleStartIdx,
                                  const index_type circleEndIdx) const
  {
    for (index_type i{circleStartIdx + 1}; i < circleEndIdx; i++)
    {
      indices.push_back(midPointIdx);
      indices.push_back(i - 1);
      indices.push_back(i);
    }
    indices.push_back(midPointIdx);
    indices.push_back(circleStartIdx);
    indices.push_back(circleEndIdx - 1);
  }

  std::vector<index_type> calc_cylinder_triangle_indices(const std::vector<linal::vec3<value_type>>& cylinderPoints) const
  {
    // Bottom circle
    std::vector<index_type> indices;
    const index_type cylPointsSize = to_idx<index_type>(cylinderPoints.size());

    const index_type bottomMidPointIdx = cylPointsSize - 2;
    const index_type bottomCircleStartIdx = 0;
    const index_type bottomCircleEndIdx = 0 + m_azimuthCount;
    calc_circle_buffer_indices(indices, bottomMidPointIdx, bottomCircleStartIdx, bottomCircleEndIdx);

    // Outer surface
    for (index_type i{1}; i < m_azimuthCount; ++i)
    {
      const index_type topIdx = i + m_azimuthCount;
      const index_type topIdxMOne = i + m_azimuthCount - 1;
      indices.push_back(i);
      indices.push_back(topIdx);
      indices.push_back(topIdxMOne);

      indices.push_back(i);
      indices.push_back(topIdxMOne);
      indices.push_back(i - 1);
    }
    const index_type topIdxStart = m_azimuthCount;
    const index_type topIdxEnd = 2 * m_azimuthCount - 1;
    indices.push_back(topIdxEnd);
    indices.push_back(topIdxStart - 1);
    indices.push_back(0);
    indices.push_back(topIdxEnd);
    indices.push_back(0);
    indices.push_back(topIdxStart);

    // Top circle
    const index_type topMidPointIdx = cylPointsSize - 1;
    const index_type topCircleStartIdx = m_azimuthCount;
    const index_type topCircleEndIdx = 2 * m_azimuthCount;
    calc_circle_buffer_indices(indices, topMidPointIdx, topCircleStartIdx, topCircleEndIdx);

    return indices;
  }
};

} // namespace Geometry

#endif // GEOMETRY_CYLINDERMESHBUILDER_HPP
