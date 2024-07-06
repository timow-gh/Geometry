#ifndef MESH_HEADER
#define MESH_HEADER

#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class MeshPoints {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;

  MeshPoints() = default;

  GEO_NODISCARD constexpr std::vector<linal::vec3<TFloat>>& getPoints() { return m_points; }
  GEO_NODISCARD constexpr const std::vector<linal::vec3<TFloat>>& getPoints() const { return m_points; }
  GEO_NODISCARD constexpr linal::vec3<TFloat> getPoint(std::size_t index) const { return m_points[index]; }
  GEO_NODISCARD constexpr std::size_t size() const { return m_points.size(); }

  constexpr void setPoints(const std::vector<linal::vec3<TFloat>>& points) { m_points = points; }

  GEO_NODISCARD constexpr VertexIndex_t add(const linal::vec3<TFloat>& vector)
  {
    m_points.push_back(vector);
    return VertexIndex_t{static_cast<TIndex>(m_points.size()) - 1};
  }

  GEO_NODISCARD constexpr bool contains(const linal::vec3<TFloat>& vector, VertexIndex_t& index)
  {
    for (TIndex i = 0; i < m_points.size(); ++i)
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
  std::vector<linal::vec3<TFloat>> m_points;
};

template <typename TFloat, typename U>
constexpr std::vector<U> vectorsToComponents(const std::vector<linal::vec3<TFloat>>& vectors)
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
