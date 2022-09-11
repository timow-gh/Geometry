#ifndef MESH_HEADER
#define MESH_HEADER

#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <cstddef>
#include <iostream>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class MeshPoints {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndexType>::VertexIndex_t;

  MeshPoints() = default;

  // TODO: Implement a kd-tree and check for existing points for large meshes
  // with a lot of insertions
  CORE_NODISCARD CORE_CONSTEXPR VertexIndex_t add(const LinAl::Vec3<TFloatType>& vector)
  {
    m_points.push_back(vector);
    return VertexIndex_t{m_points.size() - 1};
  }

  CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3Vector<TFloatType>& getPoints() { return m_points; }
  CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3Vector<TFloatType>& getPoints() const { return m_points; }

  CORE_CONSTEXPR void setPoints(const LinAl::Vec3Vector<TFloatType>& points) { m_points = points; }

  CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<TFloatType>& vector, VertexIndex_t& index)
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

  CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3<TFloatType> getPoint(std::size_t index) const { return m_points[index]; }
  CORE_NODISCARD CORE_CONSTEXPR std::size_t size() const { return m_points.size(); }

  CORE_CONSTEXPR void transform(const LinAl::Vec3<TFloatType>& translation)
  {
    for (auto& vec: m_points)
      vec += translation;
  }

private:
  LinAl::Vec3Vector<TFloatType> m_points;
};

template <typename TFloatType, typename U>
CORE_CONSTEXPR Core::TVector<U> vectorsToComponents(const LinAl::Vec3Vector<TFloatType>& vectors)
{
  Core::TVector<U> result;
  for (const LinAl::Vec3<TFloatType>& vector: vectors)
    for (std::size_t i{0}; i < 3; ++i)
    {
      result.push_back(static_cast<U>(vector[i]));
    }
  return result;
}

} // namespace Geometry

#endif