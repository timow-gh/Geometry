#ifndef MESH_HEADER
#define MESH_HEADER

#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename TMeshTraits>
class MeshPoints {
public:
  using value_type = typename TMeshTraits::value_type;
  using index_type = typename TMeshTraits::index_type;

  using VertexIndex_t = typename TMeshTraits::VertexIndex_t;
  using HalfedgeIndex_t = typename TMeshTraits::HalfedgeIndex_t;
  using FacetIndex_t = typename TMeshTraits::FacetIndex_t;

  using Vertex_t = typename TMeshTraits::Vertex_t;
  using Halfedge_t = typename TMeshTraits::Halfedge_t;
  using Facet_t = typename TMeshTraits::Facet_t;

  using HalfedgeMesh_t = typename TMeshTraits::HalfedgeMesh_t;

  MeshPoints() = default;

  GEO_NODISCARD constexpr std::vector<linal::vec3<value_type>>& getPoints() { return m_points; }
  GEO_NODISCARD constexpr const std::vector<linal::vec3<value_type>>& getPoints() const { return m_points; }
  GEO_NODISCARD constexpr linal::vec3<value_type> getPoint(std::size_t index) const { return m_points[index]; }
  GEO_NODISCARD constexpr std::size_t size() const { return m_points.size(); }

  constexpr void setPoints(const std::vector<linal::vec3<value_type>>& points) { m_points = points; }

  GEO_NODISCARD constexpr VertexIndex_t add(const linal::vec3<value_type>& vector)
  {
    m_points.push_back(vector);
    return VertexIndex_t{static_cast<index_type>(m_points.size()) - 1};
  }

  /**
   * @brief Checks if the mesh points contain a given vector.
   *
   * @param vector The vector to check for.
   * @param index If the vector is found, the index of the vector is stored here. Otherwise, the index is set to invalid.
   * @return True, if the vector is found, false otherwise.
   */
  GEO_NODISCARD constexpr bool contains(const linal::vec3<value_type>& vector, VertexIndex_t& index)
  {
    for (index_type i = 0; i < m_points.size(); ++i)
    {
      if (m_points[i] == vector)
      {
        index = VertexIndex_t{i};
        return true;
      }
    }
    index = VertexIndex_t{};
    return false;
  }

  constexpr void transform(const linal::vec3<value_type>& translation)
  {
    for (auto& vec: m_points)
      vec += translation;
  }

private:
  std::vector<linal::vec3<value_type>> m_points;
};

template <typename value_type, typename U>
constexpr std::vector<U> vectorsToComponents(const std::vector<linal::vec3<value_type>>& vectors)
{
  std::vector<U> result;
  for (const linal::vec3<value_type>& vector: vectors)
    for (std::size_t i{0}; i < 3; ++i)
    {
      result.push_back(static_cast<U>(vector[i]));
    }
  return result;
}

} // namespace Geometry

#endif
