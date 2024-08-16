#ifndef GEOMETRY_SPHEREMESHBUILDER_HPP
#define GEOMETRY_SPHEREMESHBUILDER_HPP

#include "Geometry/HalfedgeMesh/CalcTriangleIndices.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"
#include <cmath>
#include <linal/utils/constants.hpp>
#include <linal/vec.hpp>
#include <memory>
#include <optional>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class SphereMeshBuilder : public MeshBuilderBase<MeshTraits<TFloat, TIndex>, SphereMeshBuilder<TFloat, TIndex>> {
public:
  using MeshTraits_t = MeshTraits<TFloat, TIndex>;
  using value_type = typename MeshTraits_t::value_type;
  using index_type = typename MeshTraits_t::index_type;

  using HalfedgeMesh_t = typename MeshTraits_t::HalfedgeMesh_t;

  index_type m_polarCount{10};
  index_type m_azimuthCount{20};
  std::optional<Sphere<value_type>> m_sphere;

public:
  SphereMeshBuilder& set_polar_count(uint32_t polarCount)
  {
    m_polarCount = polarCount;
    return *this;
  }

  SphereMeshBuilder& set_azimuth_count(uint32_t azimuthCount)
  {
    m_azimuthCount = azimuthCount;
    return *this;
  }

  SphereMeshBuilder& set_sphere(const Sphere<value_type>& sphere)
  {
    m_sphere = sphere;
    return *this;
  }

  std::unique_ptr<HalfedgeMesh_t> build()
  {
    if (!m_sphere)
      return nullptr;

    auto spherePoints = calc_sphere_points(*m_sphere);
    auto triangleIndices = calc_sphere_triangle_indices(to_idx<index_type>(spherePoints.size()));
    return MeshBuilderBase<MeshTraits<TFloat, TIndex>, SphereMeshBuilder<TFloat, TIndex>>::build_triangle_halfedge_mesh(spherePoints,
                                                                                                                        triangleIndices);
  }

private:
  std::vector<linal::vec3<value_type>> calc_sphere_points(const Sphere<value_type>& sphere)
  {
    std::vector<linal::vec3<value_type>> points;

    value_type polarStep = linal::PI<value_type> / static_cast<value_type>(m_polarCount);
    value_type azimuthStep = value_type{2.0} * linal::PI<value_type> / static_cast<value_type>(m_azimuthCount);
    value_type radius = sphere.get_radius();

    for (uint32_t i{1}; i < m_polarCount; ++i)
    {
      value_type polarAngle = static_cast<value_type>(i) * polarStep;
      value_type z = radius * std::cos(polarAngle);
      value_type projRadius = radius * std::sin(polarAngle);

      for (uint32_t j{0}; j < m_azimuthCount; ++j)
      {
        value_type azimuthAngle = static_cast<value_type>(j) * azimuthStep;
        value_type x = projRadius * std::cos(azimuthAngle);
        value_type y = projRadius * std::sin(azimuthAngle);
        points.push_back(linal::vec3<value_type>{x, y, z});
      }
    }

    // Poles
    points.push_back(linal::vec3<value_type>{0, 0, radius});
    points.push_back(linal::vec3<value_type>{0, 0, -radius});

    const auto& sphereOrigin = sphere.get_origin();
    if (sphereOrigin != linal::vec3<value_type>{})
    {
      for (auto& point: points)
      {
        point += sphereOrigin;
      }
    }

    return points;
  }

  std::vector<index_type> calc_sphere_triangle_indices(index_type spherePointsSize)
  {
    auto toIdx = [azimuthCount = m_azimuthCount](index_type i, index_type j) -> index_type
    { return static_cast<index_type>(i * azimuthCount + j); };

    std::vector<index_type> triangleIndices;

    const index_type topiIdx = 0;
    const index_type topIdx = spherePointsSize - 2;

    const index_type bottomiIdx = m_polarCount - 2;
    const index_type bottomIdx = spherePointsSize - 1;

    // First top triangle
    triangleIndices.push_back(toIdx(topiIdx, m_azimuthCount - 1));
    triangleIndices.push_back(topIdx);
    triangleIndices.push_back(toIdx(topiIdx, 0));

    // First bottom triangle
    triangleIndices.push_back(toIdx(bottomiIdx, 0));
    triangleIndices.push_back(bottomIdx);
    triangleIndices.push_back(toIdx(bottomiIdx, m_azimuthCount - 1));

    for (index_type j{1}; j < m_azimuthCount; ++j)
    {
      index_type jIdx = toIdx(topiIdx, j);
      index_type jprevIdx = toIdx(topiIdx, j - 1);

      triangleIndices.push_back(topIdx);
      triangleIndices.push_back(jIdx);
      triangleIndices.push_back(jprevIdx);

      jIdx = toIdx(bottomiIdx, j);
      jprevIdx = toIdx(bottomiIdx, j - 1);

      triangleIndices.push_back(jIdx);
      triangleIndices.push_back(bottomIdx);
      triangleIndices.push_back(jprevIdx);
    }

    // Sphere body triangles
    for (index_type i{1}; i < m_polarCount - 1; ++i)
    {
      const index_type iprev = i - 1;
      for (index_type j{1}; j < m_azimuthCount; ++j)
      {
        const index_type jprev = j - 1;

        const index_type iprevj = toIdx(iprev, j);
        const index_type ijprev = toIdx(i, jprev);

        triangleIndices.push_back(iprevj);
        triangleIndices.push_back(ijprev);
        triangleIndices.push_back(toIdx(iprev, jprev));

        triangleIndices.push_back(ijprev);
        triangleIndices.push_back(iprevj);
        triangleIndices.push_back(toIdx(i, j));
      }

      // Triangles from first and last points in the sphere point array
      triangleIndices.push_back(toIdx(i, m_azimuthCount - 1));
      triangleIndices.push_back(toIdx(iprev, m_azimuthCount - 1));
      triangleIndices.push_back(toIdx(iprev, 0));

      triangleIndices.push_back(toIdx(i, 0));
      triangleIndices.push_back(toIdx(i, m_azimuthCount - 1));
      triangleIndices.push_back(toIdx(iprev, 0));
    }
    return triangleIndices;
  }
};
} // namespace Geometry

#endif // GEOMETRY_SPHEREMESHBUILDER_HPP
