#ifndef GEOMETRY_DEBUGHELPER_HPP
#define GEOMETRY_DEBUGHELPER_HPP

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include <iostream>
#include <linal/vec.hpp>
#include <vector>

namespace Geometry
{

template <typename TFloat>
void print_inline(std::ostream& os, linal::vec3<TFloat> vec)
{
  os << "vec: " << vec[0] << " " << vec[1] << " " << vec[2];
}

template <typename TFloat>
void print(std::ostream& os, linal::vec3<TFloat> vec)
{
  print_inline(os, vec);
  os << std::endl;
}

template <typename TFloat>
void print(std::ostream& os, std::vector<linal::vec3<TFloat>>& vec)
{
  for (const auto& v: vec)
  {
    print(os, v);
  }
}

template <typename TFloat, typename TIndex>
void print(std::ostream& os, const Vertex<TFloat, TIndex>& vertex)
{
  os << "Vertex index: " << vertex.getIndex().get_value() << "; ";
  linal::vec3<TFloat> vec = vertex.getVector();
  print_inline(std::cout, vec);
  os << std::endl;
}

template <typename TFloat, typename TIndex>
void print(std::ostream& os, const std::vector<Vertex<TFloat, TIndex>>& vertices)
{
  for (const auto& vertex: vertices)
  {
    print(os, vertex);
  }
}

template <typename TFloat, typename TIndex>
void print_inline(std::ostream& os, const Facet<TFloat, TIndex>& facet)
{
  os << "Facet halfedge index: " << facet.getHalfedge().getFacetIndex().get_value();
}

template <typename TFloat, typename TIndex>
void print(std::ostream& os, const Halfedge<TFloat, TIndex>& he)
{
  os << "Halfedge index: " << he.getFacet().getHalfedgeIndex().get_value();
  os << "; Halfedge vertex index: " << he.getVertex().getIndex().get_value();
  os << "; Opposite halfedge index: " << he.getOppositeIndex().get_value();
  os << "; Vertex vector, ";
  linal::vec3<TFloat> vec = he.getVertex().getVector();
  print_inline(os, vec);
  os << "; ";
  os << "Facet index: " << he.getFacetIndex().get_value();
  os << std::endl;
}

template <typename TFloat, typename TIndex>
void print(std::ostream& os, const std::vector<Halfedge<TFloat, TIndex>>& halfedges)
{
  for (const auto& he: halfedges)
  {
    print(os, he);
  }
}

template <typename TFloat, typename TIndex>
void print(std::ostream& os, const Facet<TFloat, TIndex>& facet)
{
  print_inline(os, facet);
  os << std::endl;
}

template <typename TFloat, typename TIndex>
void print(std::ostream& os, const std::vector<Facet<TFloat, TIndex>>& facets)
{
  for (const auto& facet: facets)
  {
    print(os, facet);
  }
  os << std::endl;
}

} // namespace Geometry

#endif // GEOMETRY_DEBUGHELPER_HPP
