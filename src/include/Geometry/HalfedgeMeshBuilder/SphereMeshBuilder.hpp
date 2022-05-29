#ifndef GEOMETRY_SPHEREMESHBUILDER_HPP
#define GEOMETRY_SPHEREMESHBUILDER_HPP

#include <Core/Math/Constants.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMesh/TriangleIndices.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <Geometry/Sphere.hpp>
#include <Geometry/Triangle.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <cmath>
#include <memory>
#include <optional>

namespace Geometry
{

template <typename T>
class SphereMeshBuilder : public MeshBuilderBase<T, SphereMeshBuilder<T>> {
    uint32_t m_polarCount{10};
    uint32_t m_azimuthCount{20};
    std::optional<Sphere<T>> m_sphere;

  public:
    SphereMeshBuilder& setPolarCount(uint32_t polarCount)
    {
        m_polarCount = polarCount;
        return *this;
    }

    SphereMeshBuilder& setAzimuthCount(uint32_t azimuthCount)
    {
        m_azimuthCount = azimuthCount;
        return *this;
    }

    SphereMeshBuilder& setSphere(const Sphere<T>& sphere)
    {
        m_sphere = sphere;
        return *this;
    }

    std::unique_ptr<HalfedgeMesh<TFloatType, TIndexType>> build()
    {
        if (!m_sphere)
            return nullptr;

        auto spherePoints = calcSpherePoints(*m_sphere);
        auto triangleIndices = calcSphereTriangleIndices(spherePoints);

        return MeshBuilderBase<T, SphereMeshBuilder<T>>::buildTriangleHeMesh(spherePoints, triangleIndices);
    }

  private:
    LinAl::Vec3Vector<T> calcSpherePoints(const Sphere<T>& sphere)
    {
        LinAl::Vec3Vector<T> points;

        T polarStep = Core::PI<T> / static_cast<double_t>(m_polarCount);
        T azimuthStep = 2.0 * Core::PI<T> / static_cast<double_t>(m_azimuthCount);
        T radius = sphere.getRadius();

        for (uint32_t i{1}; i < m_polarCount; ++i)
        {
            T polarAngle = i * polarStep;
            T z = radius * std::cos(polarAngle);
            T projRadius = radius * std::sin(polarAngle);

            for (uint32_t j{0}; j < m_azimuthCount; ++j)
            {
                T azimuthAngle = j * azimuthStep;
                T x = projRadius * std::cos(azimuthAngle);
                T y = projRadius * std::sin(azimuthAngle);
                points.push_back(LinAl::Vec3<T>{x, y, z});
            }
        }

        // Poles
        points.push_back(LinAl::Vec3<T>{0, 0, radius});
        points.push_back(LinAl::Vec3<T>{0, 0, -radius});

        const auto& sphereOrigin = sphere.getOrigin();
        if (sphereOrigin != LinAl::ZERO_VEC3D)
            for (auto& point: points)
                point += sphereOrigin;

        return points;
    }

    Core::TVector<uint32_t> calcSphereTriangleIndices(const LinAl::Vec3Vector<T>& spherePoints)
    {
        auto toIdx = [azimuthCount = m_azimuthCount](uint32_t i, uint32_t j) -> uint32_t
        {
            return static_cast<uint32_t>(i * azimuthCount + j);
        };

        Core::TVector<uint32_t> triangleIndices;

        // TODO (Safe conversion DebugAssert) Does spherePoints.size() fit into uint32_t
        const uint32_t pointsSize = static_cast<uint32_t>(spherePoints.size());

        const uint32_t topiIdx = 0;
        const uint32_t topIdx = pointsSize - 2;

        const uint32_t bottomiIdx = m_polarCount - 2;
        const uint32_t bottomIdx = pointsSize - 1;

        // First top triangle
        triangleIndices.push_back(topIdx);
        triangleIndices.push_back(toIdx(topiIdx, 0));
        triangleIndices.push_back(toIdx(topiIdx, m_azimuthCount - 1));
        // First bottom triangle
        triangleIndices.push_back(bottomIdx);
        triangleIndices.push_back(toIdx(bottomiIdx, m_azimuthCount - 1));
        triangleIndices.push_back(toIdx(bottomiIdx, 0));
        for (uint32_t j{1}; j < m_azimuthCount; ++j)
        {
            uint32_t jIdx = toIdx(topiIdx, j);
            uint32_t jprevIdx = toIdx(topiIdx, j - 1);

            triangleIndices.push_back(topIdx);
            triangleIndices.push_back(jprevIdx);
            triangleIndices.push_back(jIdx);

            jIdx = toIdx(bottomiIdx, j);
            jprevIdx = toIdx(bottomiIdx, j - 1);

            triangleIndices.push_back(jIdx);
            triangleIndices.push_back(jprevIdx);
            triangleIndices.push_back(bottomIdx);
        }

        // Sphere body triangles
        for (uint32_t i{1}; i < m_polarCount - 1; ++i)
        {
            const uint32_t iprev = i - 1;
            for (uint32_t j{1}; j < m_azimuthCount; ++j)
            {
                const uint32_t jprev = j - 1;

                const uint32_t iprevj = toIdx(iprev, j);
                const uint32_t ijprev = toIdx(i, jprev);

                triangleIndices.push_back(iprevj);
                triangleIndices.push_back(toIdx(iprev, jprev));
                triangleIndices.push_back(ijprev);

                triangleIndices.push_back(ijprev);
                triangleIndices.push_back(toIdx(i, j));
                triangleIndices.push_back(iprevj);
            }

            // Triangles from first and last points in the sphere point array
            triangleIndices.push_back(toIdx(i, m_azimuthCount - 1));
            triangleIndices.push_back(toIdx(iprev, 0));
            triangleIndices.push_back(toIdx(iprev, m_azimuthCount - 1));

            triangleIndices.push_back(toIdx(i, 0));
            triangleIndices.push_back(toIdx(iprev, 0));
            triangleIndices.push_back(toIdx(i, m_azimuthCount - 1));
        }
        return triangleIndices;
    }
};
} // namespace Geometry

#endif // GEOMETRY_SPHEREMESHBUILDER_HPP
