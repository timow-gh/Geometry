#ifndef GEOMETRY_CYLINDERMESHBUILDER_HPP
#define GEOMETRY_CYLINDERMESHBUILDER_HPP

#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/DiscretizeCircle.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>
#include <linal/hmat_rotation.hpp>
#include <linal/hmat_translation.hpp>
#include <linal/hvec.hpp>
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
    if (!m_cylinder)
      return nullptr;

    const auto cylinderSeg = m_cylinder->get_segment();

    linal::hcoord::hmatd hTrafo = linal::hcoord::rot_align(linal::hcoord::Z_HVECD, linal::hcoord::vec_to_hvec(cylinderSeg.direction()));
    linal::hcoord::set_translation(hTrafo, cylinderSeg.get_source());
    MeshBuilderBase<TFloat, TIndex, CylinderMeshBuilder<TFloat, TIndex>>::set_transformation(hTrafo);

    linal::vec3vector<TFloat> cylPoints = calc_cylinder_points();
    const auto cylinderTriangleIndices = calc_cylinder_triangle_indices(cylPoints);
    return MeshBuilderBase<TFloat, TIndex, CylinderMeshBuilder<TFloat, TIndex>>::build_triangle_halfedge_mesh(cylPoints,
                                                                                                              cylinderTriangleIndices);
  }

private:
  linal::vec3vector<TFloat> calc_cylinder_points() const
  {
    linal::vec3vector<TFloat> points;
    points.reserve(2 * m_azimuthCount + 2);

    const Segment3<TFloat>& segment = m_cylinder->get_segment();
    discretize_circle(points, m_cylinder->get_radius(), m_azimuthCount);

    std::size_t circlePointsSize = points.size();
    double segLength = segment.length();
    for (std::size_t i{0}; i < circlePointsSize; ++i)
    {
      points.push_back(points[i]);
      points.back()[2] += segLength;
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

  std::vector<TIndex> calc_cylinder_triangle_indices(const linal::vec3vector<TFloat>& cylinderPoints) const
  {
    // Bottom circle
    std::vector<TIndex> indices;
    const std::size_t cylPointsSize = cylinderPoints.size();

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
    const std::size_t topMidPointIdx = cylPointsSize - 1;
    const std::size_t topCircleStartIdx = m_azimuthCount;
    const std::size_t topCircleEndIdx = 2 * m_azimuthCount;
    calc_circle_buffer_indices(indices, topMidPointIdx, topCircleStartIdx, topCircleEndIdx);

    return indices;
  }
};

} // namespace Geometry

#endif // GEOMETRY_CYLINDERMESHBUILDER_HPP
