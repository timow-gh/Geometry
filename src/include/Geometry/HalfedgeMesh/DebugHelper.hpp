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

template <typename TMeshTraits>
void print(std::ostream& os, const Vertex<TMeshTraits>& vertex)
{
  using value_type = typename TMeshTraits::value_type;

  os << "Vertex index: " << vertex.getIndex().get_value() << "; ";
  linal::vec3<value_type> vec = vertex.getVector();
  print_inline(os, vec);
  os << std::endl;
}

template <typename TMeshTraits>
void print(std::ostream& os, const std::vector<Vertex<TMeshTraits>>& vertices)
{
  for (const auto& vertex: vertices)
  {
    print(os, vertex);
  }
}

template <typename TMeshTraits>
void print_inline(std::ostream& os, const Facet<TMeshTraits>& facet)
{
  os << "Facet halfedge index: " << facet.getHalfedge().getFacetIndex().get_value();
}

template <typename TMeshTraits>
void print(std::ostream& os, const Facet<TMeshTraits>& facet)
{
  print_inline(os, facet);
  os << std::endl;
}

template <typename TMeshTraits>
void print(std::ostream& os, const std::vector<Facet<TMeshTraits>>& facets)
{
  for (const auto& facet: facets)
  {
    print(os, facet);
  }
}

template <typename TMeshTraits>
void print(std::ostream& os, const Halfedge<TMeshTraits>& he)
{
  using value_type = typename TMeshTraits::value_type;

  os << "Halfedge index: " << he.getFacet().getHalfedgeIndex().get_value();
  os << "; Halfedge vertex index: " << he.getVertex().getIndex().get_value();
  os << "; Opposite halfedge index: " << he.getOppositeIndex().get_value();
  os << "; Vertex vector, ";
  linal::vec3<value_type> vec = he.getVertex().getVector();
  print_inline(os, vec);
  os << "; ";
  os << "Facet index: " << he.getFacetIndex().get_value();
  os << std::endl;
}

template <typename TMeshTraits>
void print(std::ostream& os, const std::vector<Halfedge<TMeshTraits>>& halfedges)
{
  for (const auto& he: halfedges)
  {
    print(os, he);
  }
}

} // namespace Geometry

#endif // GEOMETRY_DEBUGHELPER_HPP
