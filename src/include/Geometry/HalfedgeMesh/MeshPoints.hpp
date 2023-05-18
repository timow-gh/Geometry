#ifndef MESH_HEADER
#define MESH_HEADER

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <cstddef>
#include <iostream>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class MeshPoints {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;

  MeshPoints() = default;

  // TODO: Implement a kd-tree and check for existing points for large meshes
  // with a lot of insertions
  GEO_NODISCARD GEO_CONSTEXPR VertexIndex_t add(const LinAl::Vec3<TFloat>& vector)
  {
    m_points.push_back(vector);
    return VertexIndex_t{m_points.size() - 1};
  }

  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3Vector<TFloat>& getPoints() { return m_points; }
  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3Vector<TFloat>& getPoints() const { return m_points; }

  GEO_CONSTEXPR void setPoints(const LinAl::Vec3Vector<TFloat>& points) { m_points = points; }

  GEO_NODISCARD GEO_CONSTEXPR bool contains(const LinAl::Vec3<TFloat>& vector, VertexIndex_t& index)
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

  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3<TFloat> getPoint(std::size_t index) const { return m_points[index]; }
  GEO_NODISCARD GEO_CONSTEXPR std::size_t size() const { return m_points.size(); }

  GEO_CONSTEXPR void transform(const LinAl::Vec3<TFloat>& translation)
  {
    for (auto& vec: m_points)
      vec += translation;
  }

private:
  LinAl::Vec3Vector<TFloat> m_points;
};

template <typename TFloat, typename U>
GEO_CONSTEXPR Core::TVector<U> vectorsToComponents(const LinAl::Vec3Vector<TFloat>& vectors)
{
  Core::TVector<U> result;
  for (const LinAl::Vec3<TFloat>& vector: vectors)
    for (std::size_t i{0}; i < 3; ++i)
    {
      result.push_back(static_cast<U>(vector[i]));
    }
  return result;
}

} // namespace Geometry

#endif