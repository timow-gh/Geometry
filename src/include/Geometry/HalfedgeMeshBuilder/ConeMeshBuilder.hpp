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
class ConeMeshBuilder : public MeshBuilderBase<MeshTraits<TFloat, TIndex>, ConeMeshBuilder<TFloat, TIndex>> {
public:
  using MeshTraits_t = MeshTraits<TFloat, TIndex>;
  using HalfedgeMesh_t = HalfedgeMesh<MeshTraits_t>;
  using value_type = typename MeshTraits_t::value_type;
  using index_type = typename MeshTraits_t::index_type;

  using Cone_t = Cone<value_type>;

private:
  std::optional<Cone_t> m_cone;
  index_type m_azimuthCount{20};

public:
  ConeMeshBuilder() = default;

  ConeMeshBuilder& set_cone(const Cone_t& cone)
  {
    m_cone = cone;
    return *this;
  }

  ConeMeshBuilder& set_azimuth_count(index_type azimuthCount)
  {
    m_azimuthCount = azimuthCount;
    return *this;
  }

  GEO_NODISCARD std::unique_ptr<HalfedgeMesh_t> build()
  {
    GEO_ASSERT(m_cone);
    if (!m_cone)
    {
      return nullptr;
    }

    const auto coneSeg = m_cone->get_segment();
    GEO_ASSERT(!linal::isZero(coneSeg.length()));

    const auto& dir = coneSeg.direction();
    GEO_ASSERT(linal::isEq(linal::length(dir), value_type{1.0}));

    linal::hmat<value_type> hTrafo;
    linal::rot_align(hTrafo, linal::hvec<value_type>{0.0, 0.0, 1.0, 1.0}, linal::hvec<value_type>{dir[0], dir[1], dir[2], 1.0});
    hTrafo.set_translation(coneSeg.get_source());
    MeshBuilderBase<MeshTraits_t, ConeMeshBuilder<value_type, index_type>>::set_transformation(hTrafo);

    std::vector<linal::vec3<value_type>> conePoints;
    std::vector<index_type> coneTriangleIndices;
    calc_cone_triangles(conePoints, coneTriangleIndices);

    return MeshBuilderBase<MeshTraits_t, ConeMeshBuilder<value_type, index_type>>::build_triangle_halfedge_mesh(conePoints,
                                                                                                                coneTriangleIndices);
  }

private:
  void calc_cone_triangles(std::vector<linal::vec3<value_type>>& conePoints, std::vector<index_type>& coneTriangleIndices) const
  {
    GEO_ASSERT(m_cone);
    GEO_ASSERT(conePoints.empty());
    GEO_ASSERT(coneTriangleIndices.empty());

    // Bottom circle triangles and indices.
    calc_circle_triangles(
        linal::vec3<value_type>{0, 0, 0}, // The ConeMeshBuilder assumes the cone to be aligned with the z-axis and placed at the origin.
        m_cone->get_radius(),
        m_azimuthCount,
        CircleDiscretizationDirection::CCW,
        std::back_inserter(conePoints),
        index_type{0},
        std::back_inserter(coneTriangleIndices));

    // Mantle triangles.
    conePoints.push_back(linal::vec3<value_type>{0, 0, m_cone->get_segment().length()});
    index_type circlePointsStartIdx{1}; // The first point is the center of the bottom circle.
    index_type circlePointsEndIdx = static_cast<index_type>(conePoints.size() - 2);
    index_type peakIdx = static_cast<index_type>(conePoints.size() - 1);
    for (index_type i{circlePointsStartIdx}; i <= circlePointsEndIdx; ++i)
    {
      coneTriangleIndices.push_back(peakIdx);
      coneTriangleIndices.push_back(i == circlePointsEndIdx ? circlePointsStartIdx : i + 1);
      coneTriangleIndices.push_back(i);
    }
  }
};

} // namespace Geometry

#endif // GEOMETRY_CONEMESHBUILDER_HPP
