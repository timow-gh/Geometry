#ifndef GEOMETRY_MESHTRIANGLEADDER_HPP
#define GEOMETRY_MESHTRIANGLEADDER_HPP

#include "Geometry/HalfedgeMesh/ConversionHelper.hpp"
#include "Geometry/HalfedgeMesh/DebugHelper.hpp"
#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/KdTree.hpp"
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
    GEO_ASSERT(std::all_of(vertexIndices.begin(), vertexIndices.end(), [](const auto& index) { return index.is_valid(); }));

    FacetIndex_t facetIdx;
    for (std::size_t i = 0; i < 3; ++i)
    {
      HalfedgeIndex_t halfedgeIndex{calc_next_index(halfedges.size())};
      GEO_ASSERT(halfedgeIndex.is_valid());

      if (i == 0)
      {
        facetIdx = FacetIndex_t{to_idx<TIndex>(facets.size())};
        facets.emplace_back(halfedgeIndex, *m_halfedgeMesh);
        GEO_ASSERT(facetIdx.is_valid());
      }

      const VertexIndex_t& vIndex = vertexIndices[i];
      GEO_ASSERT(vIndex.is_valid());

      halfedges.emplace_back(halfedgeIndex, vIndex, m_halfedgeMesh);
      GEO_ASSERT(halfedges.back().getHalfedgeIndex().is_valid());

      GEO_ASSERT(vIndex.get_value() < vertices.size());
      auto& heVertex = vertices[vIndex.get_value()];
      heVertex.addHalfedgeIndex(halfedgeIndex);
    }
    GEO_ASSERT(facetIdx.is_valid());

    GEO_ASSERT(halfedges.size() >= 3);
    auto startIdx = to_idx<TIndex>(halfedges.size() - 3);
    auto endIdx = to_idx<TIndex>(halfedges.size());
    auto lastElemIdx = to_idx<TIndex>(endIdx - 1);

    for (TIndex i = startIdx; i < endIdx; ++i)
    {
      halfedges[i].setFacetIndex(facetIdx);

      TIndex nextHeIndex = (i == lastElemIdx) ? startIdx : i + 1;
      halfedges[i].setNextIndex(HalfedgeIndex_t{nextHeIndex});

      TIndex previousHeIndex = (i == startIdx) ? to_idx<TIndex>(lastElemIdx) : i - 1;
      halfedges[i].setPreviousIndex(HalfedgeIndex_t{previousHeIndex});
    }
  }

  static void set_opposite_halfedges(HalfedgeMesh<TFloat, TIndex>& halfedgeMesh)
  {
    std::vector<Halfedge_t>& halfedges = halfedgeMesh.getHalfedges();

    set_opposite_halfedges(halfedges);
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
        vertices.emplace_back(VertexIndex_t{to_idx<TIndex>(vertices.size())}, m_halfedgeMesh);
      }
      GEO_ASSERT(vertexIndex.is_valid());
      vertexIndices[i] = vertexIndex;
    }
  }

  static void set_opposite_halfedges(std::vector<Halfedge_t>& halfedges)
  {
    // TODO Ensure that the halfedge indidces are unique
    KdTree<linal::vec3<TFloat>, std::vector<std::size_t>> kdTree; // Stores all halfedges with their start vertex
    for (std::size_t i = 0; i < halfedges.size(); ++i)
    {
      linal::vec3<TFloat> start = halfedges[i].getVertex().getVector();

      bool exists = kdTree.search(start);
      if (exists)
      {
        std::pair<linal::vec3<TFloat>, std::vector<std::size_t>&> res = kdTree.nearest(start);
        res.second.push_back(i);
      }
      else
      {
        std::vector<std::size_t> hes;
        hes.push_back(i);
        kdTree.insert(start, hes);
      }
    }

    // For all halfedges, find the opposite halfedge
    for (auto& he: halfedges)
    {
      linal::vec3<TFloat> start = he.getNextVertex().getVector();
      linal::vec3<TFloat> end = he.getVertex().getVector();
      std::pair<linal::vec3<TFloat>, std::vector<std::size_t>&> result = kdTree.nearest(end);
      [[maybe_unused]] bool foundOpposite = false;
      for (std::size_t oppHeIndex: result.second)
      {
        auto& oppositeHeCandidate = halfedges[oppHeIndex];
        if (oppositeHeCandidate.getNextVertex().getVector() == start)
        {
          he.setOppositeIndex(oppositeHeCandidate.getHalfedgeIndex());
          oppositeHeCandidate.setOppositeIndex(he.getHalfedgeIndex());
          foundOpposite = true;
          break;
        }
      }
      GEO_ASSERT(foundOpposite);
    }
  }
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRIANGLEADDER_HPP
