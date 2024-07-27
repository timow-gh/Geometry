#ifndef ORIGAMI_CALCVERTEXNORMALS_HPP
#define ORIGAMI_CALCVERTEXNORMALS_HPP

#include "Geometry/HalfedgeMesh/CalcFacetNormal.hpp"
#include "Geometry/HalfedgeMesh/CalcVertexFacets.hpp"
#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include <linal/vec.hpp>
#include <vector>

namespace Geometry
{

template <typename TFloat, typename TIndex>
std::vector<linal::vec3<TFloat>> calc_vertex_normals(const HalfedgeMesh<TFloat, TIndex>& mesh)
{
  using vec_t = linal::vec3<TFloat>;
  std::vector<vec_t> normals;
  normals.reserve(mesh.getVertices().size());

  using vertex_t = Vertex<TFloat, TIndex>;

  for (const vertex_t& vertex: mesh.getVertices())
  {
    const std::vector<Facet<TFloat, TIndex>> facets = calc_vertex_facets(vertex);

    std::vector<vec_t> facetNormals;
    for (const Facet<TFloat, TIndex>& facet: facets)
    {
      facetNormals.push_back(linal::normalize(calc_facet_normal(facet)));
    }
    std::sort(facetNormals.begin(),
              facetNormals.end(),
              [](const vec_t& a, const vec_t& b)
              { return a[0] < b[0] || (a[0] == b[0] && a[1] < b[1]) || (a[0] == b[0] && a[1] == b[1] && a[2] < b[2]); });
    auto iter = std::unique(facetNormals.begin(), facetNormals.end());
    facetNormals.erase(iter, facetNormals.end());

    vec_t vertexNormal{0, 0, 0};
    for (const vec_t& normal: facetNormals)
    {
      vertexNormal += normal;
    }

    normals.push_back(linal::normalize(vertexNormal));
  }

  return normals;
}

} // namespace Geometry

#endif // ORIGAMI_CALCVERTEXNORMALS_HPP
