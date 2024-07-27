#ifndef GEOMETRY_MESHTRIANGLEADDER_HPP
#define GEOMETRY_MESHTRIANGLEADDER_HPP


#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/Triangle.hpp"
#include "Geometry/Utils/Assert.hpp"
#include <iostream>

namespace Geometry
{

/**
 * @brief Adds a triangle to a halfedge mesh.
 *
 * @tparam TFloat
 * @tparam TIndex
 */
template <typename TFloat, typename TIndex>
class MeshTriangleAdder {
  HalfedgeMesh<TFloat, TIndex>* m_halfedgeMesh;

public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;
  using FacetIndex_t = typename MeshIndexTraits<TIndex>::FacetIndex_t;

  using Vertex_t = Vertex<TFloat, TIndex>;
  using Halfedge_t = Halfedge<TFloat, TIndex>;
  using Facet_t = Facet<TFloat, TIndex>;

  using MeshPoints_t = MeshPoints<TFloat, TIndex>;

  constexpr explicit MeshTriangleAdder(HalfedgeMesh<TFloat, TIndex>& halfedgeMesh) noexcept
      : m_halfedgeMesh(&halfedgeMesh)
  {
  }

  void operator()(const Triangle3<TFloat>& triangle)
  {
    MeshPoints_t& meshPoints = m_halfedgeMesh->getMeshPoints();
    std::vector<Vertex_t>& vertices = m_halfedgeMesh->getVertices();
    std::vector<Halfedge_t>& halfedges = m_halfedgeMesh->getHalfedges();
    std::vector<Facet_t>& facets = m_halfedgeMesh->getFacets();

    std::array<VertexIndex_t, 3> vertexIndices;
    create_or_find_vertex(triangle, meshPoints, vertices, vertexIndices);
    GEO_ASSERT(vIndex.is_valid());

    FacetIndex_t facetIdx;
    for (std::size_t i = 0; i < 3; ++i)
    {
      HalfedgeIndex_t halfedgeIndex{calc_next_index(halfedges.size())};
      GEO_ASSERT(halfedgeIndex.is_valid());

      if (i == 0)
      {
        facetIdx = FacetIndex_t{facets.size()};
        facets.emplace_back(halfedgeIndex, *m_halfedgeMesh);
        GEO_ASSERT(facetIdx.is_valid());
      }

      const VertexIndex_t& vIndex = vertexIndices[i];
      GEO_ASSERT(vIndex.is_valid());

      halfedges.emplace_back(halfedgeIndex, vIndex, m_halfedgeMesh);

      GEO_ASSERT(vIndex.get_value() < vertices.size());
      vertices[vIndex.get_value()].addHalfedgeIndex(halfedgeIndex);
    }
    GEO_ASSERT(he.getHalfedgeIndex().is_valid());
    GEO_ASSERT(facetIdx.is_valid());

    GEO_ASSERT(halfedges.size() >= 3);
    std::size_t startIdx = halfedges.size() - 3;
    std::size_t endIdx = halfedges.size();
    std::size_t lastElemIdx = endIdx - 1;

    for (TIndex i = startIdx; i < endIdx; ++i)
    {
      halfedges[i].setFacetIndex(facetIdx);

      TIndex nextHeIndex = (i == lastElemIdx) ? startIdx : i + 1;
      halfedges[i].setNextIndex(HalfedgeIndex_t{nextHeIndex});

      TIndex previousHeIndex = (i == startIdx) ? static_cast<TIndex>(lastElemIdx) : i - 1;
      halfedges[i].setPreviousIndex(HalfedgeIndex_t{previousHeIndex});
    }

    set_vertex_halfedges(vertices, halfedges);
  }

  static void set_opposite_halfedges(HalfedgeMesh<TFloat, TIndex>& halfedgeMesh)
  {
    std::vector<Halfedge_t>& halfedges = halfedgeMesh.getHalfedges();

    set_opposite_halfedges(halfedges);
  }

  static void set_vertex_halfedges(HalfedgeMesh<TFloat, TIndex>& halfedgeMesh)
  {
    std::vector<Vertex_t>& vertices = halfedgeMesh.getVertices();
    std::vector<Halfedge_t>& halfedges = halfedgeMesh.getHalfedges();

    set_vertex_halfedges(vertices, halfedges);
  }

private:
  auto calc_next_index(const std::size_t size) const { return static_cast<typename HalfedgeIndex_t::value_type>(size); }

  /** @brief Create or find the Vertex of the given triangle points
   *
   * @param triangle The triangle to create or find the Vertices for.
   * @param meshPoints The already existing MeshPoints.
   * @param vertices The already existing Vertices.
   * @param vertexIndices New or found Vertex indices for the triangle points.
   */
  void create_or_find_vertex(const Triangle<TFloat, 3>& triangle,
                             MeshPoints_t& meshPoints,
                             std::vector<Vertex_t>& vertices,
                             std::array<VertexIndex_t, 3>& vertexIndices) const
  {
    // Create or find the Vertex of the linal::vec3
    const auto& trianglePoints = triangle.get_triangle_points();
    for (std::size_t i = 0; i < 3; ++i)
    {
      VertexIndex_t vertexIndex{};
      if (!meshPoints.contains(trianglePoints[i], vertexIndex))
      {
        vertexIndex = meshPoints.add(trianglePoints[i]);
        vertices.emplace_back(VertexIndex_t{vertices.size()}, m_halfedgeMesh);
      }
      GEO_ASSERT(vertexIndex.is_valid());
      vertexIndices[i] = vertexIndex;
    }
  }

  // TODO This is O(n^2) and should be optimized
  static void set_opposite_halfedges(std::vector<Halfedge_t>& halfedges)
  {
    for (std::size_t i = 0; i < halfedges.size(); ++i)
    {
      for (std::size_t j = 0; j < halfedges.size(); ++j)
      {
        auto& he1 = halfedges[i];
        auto& he2 = halfedges[j];

        if (he1 == he2)
        {
          continue;
        }

        linal::vec3<TFloat> he1Start = he1.getVertex().getVector();

        linal::vec3<TFloat> he1End = he1.getNextVertex().getVector();

        linal::vec3<TFloat> he2Start = he2.getVertex().getVector();
        linal::vec3<TFloat> he2End = he2.getNextVertex().getVector();

        if (linal::is_equal(he1Start, he2End) && linal::is_equal(he1End, he2Start))
        {
          he1.setOppositeIndex(HalfedgeIndex_t{static_cast<typename HalfedgeIndex_t::value_type>(j)});
          he2.setOppositeIndex(HalfedgeIndex_t{static_cast<typename HalfedgeIndex_t::value_type>(i)});
          break;
        }
      }
    }
  }

  static void set_vertex_halfedges(std::vector<Vertex_t>& vertices, std::vector<Halfedge_t>& halfedges)
  {
    for (Vertex_t& vertex: vertices)
    {
      for (const auto& he: halfedges)
      {
        if (he.getVertexIndex() == vertex.getIndex())
        {
          bool found = false;
          for (const auto& vertexHeIndex: vertex.getHalfedgeIndices())
          {
            if (vertexHeIndex == he.getHalfedgeIndex())
            {
              found = true;
              break;
            }
          }

          if (!found)
          {
            vertex.addHalfedgeIndex(he.getHalfedgeIndex());
          }
        }
      }
    }
  }
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRIANGLEADDER_HPP
