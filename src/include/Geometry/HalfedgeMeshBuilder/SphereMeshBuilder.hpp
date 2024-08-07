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
class SphereMeshBuilder : public MeshBuilderBase<TFloat, TIndex, SphereMeshBuilder<TFloat, TIndex>> {
  TIndex m_polarCount{10};
  TIndex m_azimuthCount{20};
  std::optional<Sphere<TFloat>> m_sphere;

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

  SphereMeshBuilder& set_sphere(const Sphere<TFloat>& sphere)
  {
    m_sphere = sphere;
    return *this;
  }

  std::unique_ptr<HalfedgeMesh<TFloat, TIndex>> build()
  {
    if (!m_sphere)
      return nullptr;

    auto spherePoints = calc_sphere_points(*m_sphere);
    auto triangleIndices = calc_sphere_triangle_indices(to_idx<TIndex>(spherePoints.size()));
    return MeshBuilderBase<TFloat, TIndex, SphereMeshBuilder<TFloat, TIndex>>::build_triangle_halfedge_mesh(spherePoints, triangleIndices);
  }

private:
  std::vector<linal::vec3<TFloat>> calc_sphere_points(const Sphere<TFloat>& sphere)
  {
    std::vector<linal::vec3<TFloat>> points;

    TFloat polarStep = linal::PI<TFloat> / static_cast<TFloat>(m_polarCount);
    TFloat azimuthStep = TFloat{2.0} * linal::PI<TFloat> / static_cast<TFloat>(m_azimuthCount);
    TFloat radius = sphere.get_radius();

    for (uint32_t i{1}; i < m_polarCount; ++i)
    {
      TFloat polarAngle = static_cast<TFloat>(i) * polarStep;
      TFloat z = radius * std::cos(polarAngle);
      TFloat projRadius = radius * std::sin(polarAngle);

      for (uint32_t j{0}; j < m_azimuthCount; ++j)
      {
        TFloat azimuthAngle = static_cast<TFloat>(j) * azimuthStep;
        TFloat x = projRadius * std::cos(azimuthAngle);
        TFloat y = projRadius * std::sin(azimuthAngle);
        points.push_back(linal::vec3<TFloat>{x, y, z});
      }
    }

    // Poles
    points.push_back(linal::vec3<TFloat>{0, 0, radius});
    points.push_back(linal::vec3<TFloat>{0, 0, -radius});

    const auto& sphereOrigin = sphere.get_origin();
    if (sphereOrigin != linal::vec3<TFloat>{})
    {
      for (auto& point: points)
      {
        point += sphereOrigin;
      }
    }

    return points;
  }

  std::vector<TIndex> calc_sphere_triangle_indices(TIndex spherePointsSize)
  {
    auto toIdx = [azimuthCount = m_azimuthCount](TIndex i, TIndex j) -> TIndex { return static_cast<TIndex>(i * azimuthCount + j); };

    std::vector<TIndex> triangleIndices;

    const TIndex topiIdx = 0;
    const TIndex topIdx = spherePointsSize - 2;

    const TIndex bottomiIdx = m_polarCount - 2;
    const TIndex bottomIdx = spherePointsSize - 1;

    // First top triangle
    triangleIndices.push_back(toIdx(topiIdx, m_azimuthCount - 1));
    triangleIndices.push_back(topIdx);
    triangleIndices.push_back(toIdx(topiIdx, 0));

    // First bottom triangle
    triangleIndices.push_back(toIdx(bottomiIdx, 0));
    triangleIndices.push_back(bottomIdx);
    triangleIndices.push_back(toIdx(bottomiIdx, m_azimuthCount - 1));

    for (TIndex j{1}; j < m_azimuthCount; ++j)
    {
      TIndex jIdx = toIdx(topiIdx, j);
      TIndex jprevIdx = toIdx(topiIdx, j - 1);

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
    for (TIndex i{1}; i < m_polarCount - 1; ++i)
    {
      const TIndex iprev = i - 1;
      for (TIndex j{1}; j < m_azimuthCount; ++j)
      {
        const TIndex jprev = j - 1;

        const TIndex iprevj = toIdx(iprev, j);
        const TIndex ijprev = toIdx(i, jprev);

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
