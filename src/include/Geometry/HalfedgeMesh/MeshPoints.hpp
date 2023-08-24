#ifndef MESH_HEADER
#define MESH_HEADER

#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <linal/containers.hpp>
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class MeshPoints {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;

  MeshPoints() = default;

  GEO_NODISCARD constexpr linal::vec3vector<TFloat>& getPoints() { return m_points; }
  GEO_NODISCARD constexpr const linal::vec3vector<TFloat>& getPoints() const { return m_points; }
  GEO_NODISCARD constexpr linal::vec3<TFloat> getPoint(std::size_t index) const { return m_points[index]; }
  GEO_NODISCARD constexpr std::size_t size() const { return m_points.size(); }

  constexpr void setPoints(const linal::vec3vector<TFloat>& points) { m_points = points; }

  GEO_NODISCARD constexpr VertexIndex_t add(const linal::vec3<TFloat>& vector)
  {
    m_points.push_back(vector);
    return VertexIndex_t{m_points.size() - 1};
  }

  GEO_NODISCARD constexpr bool contains(const linal::vec3<TFloat>& vector, VertexIndex_t& index)
  {
    for (std::size_t i = 0; i < m_points.size(); ++i)
    {
      if (m_points[i] == vector)
      {
        index = VertexIndex_t{i};
        return true;
      }
    }
    return false;
  }

  constexpr void transform(const linal::vec3<TFloat>& translation)
  {
    for (auto& vec: m_points)
      vec += translation;
  }

private:
  linal::vec3vector<TFloat> m_points;
};

template <typename TFloat, typename U>
constexpr std::vector<U> vectorsToComponents(const linal::vec3vector<TFloat>& vectors)
{
  std::vector<U> result;
  for (const linal::vec3<TFloat>& vector: vectors)
    for (std::size_t i{0}; i < 3; ++i)
    {
      result.push_back(static_cast<U>(vector[i]));
    }
  return result;
}

} // namespace Geometry

#endif
