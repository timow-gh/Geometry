#ifndef GEOMETRY_SPHEREMESHBUILDER_HPP
#define GEOMETRY_SPHEREMESHBUILDER_HPP

#include "Core/Math/Constants.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <cmath>
#include <memory>
#include <optional>

namespace Geometry
{

template <typename T>
class SphereMeshBuilder
{
    std::size_t m_polarCount{10};
    std::size_t m_azimuthCount{20};
    std::optional<Sphere<T>> m_sphere;

  public:
    SphereMeshBuilder& setPolarCount(std::size_t polarCount)
    {
        m_polarCount = polarCount;
        return *this;
    }

    SphereMeshBuilder& setAzimuthCount(std::size_t azimuthCount)
    {
        m_azimuthCount = azimuthCount;
        return *this;
    }

    SphereMeshBuilder& setSphere(const Sphere<T>& sphere)
    {
        m_sphere = sphere;
        return *this;
    }

    std::unique_ptr<HalfedgeMesh<T>> build()
    {
        if (!m_sphere)
            return nullptr;

        auto heMesh = std::make_unique<HalfedgeMesh<T>>(xg::newGuid());

        auto spherePoints = calcSpherePoints(*m_sphere);
        auto triangleIndices = calcSphereTriangleIndices(spherePoints);

        std::size_t size = triangleIndices.size();
        for (std::size_t i{2}; i < size; i += 3)
        {
            std::size_t a = triangleIndices[i - 2];
            std::size_t b = triangleIndices[i - 1];
            std::size_t c = triangleIndices[i];

            addTriangle(heMesh.get(),
                        Triangle<T, 3>(spherePoints[a],
                                       spherePoints[b],
                                       spherePoints[c]));
        }

        return heMesh;
    }

  private:
    LinAl::Vec3Vector<T> calcSpherePoints(const Sphere<T>& sphere)
    {
        LinAl::Vec3Vector<T> points;

        T polarStep = Core::PI / static_cast<double_t>(m_polarCount);
        T azimuthStep = 2.0 * Core::PI / static_cast<double_t>(m_azimuthCount);
        T radius = m_sphere->getRadius();

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

        points.push_back(LinAl::Vec3<T>{0, 0, radius});
        points.push_back(LinAl::Vec3<T>{0, 0, -radius});

        return points;
    }

    Core::TVector<uint32_t>
    calcSphereTriangleIndices(const LinAl::Vec3Vector<T>& spherePoints)
    {
        auto toIdx = [azimuthCount =
                          m_azimuthCount](std::size_t i,
                                          std::size_t j) -> std::size_t
        {
            return i * azimuthCount + j;
        };

        Core::TVector<uint32_t> triangleIndices;

        // Top and bottom triangles
        const std::size_t pointsSize = spherePoints.size();

        const std::size_t topiIdx = 0;
        const std::size_t topIdx = pointsSize - 2;

        const std::size_t bottomiIdx = m_polarCount - 1;
        const std::size_t bottomIdx = pointsSize - 1;

        // First top triangle
        //        triangleIndices.push_back(topIdx);
        //        triangleIndices.push_back(toIdx(topiIdx, 0));
        //        triangleIndices.push_back(toIdx(topiIdx, m_azimuthCount - 1));
        // First bottom triangle
        triangleIndices.push_back(bottomIdx);
        triangleIndices.push_back(toIdx(bottomiIdx, m_azimuthCount - 1));
        triangleIndices.push_back(toIdx(bottomiIdx, 0));
        for (std::size_t j{1}; j < m_azimuthCount; ++j)
        {
            //            if (j % 2)
            //                continue;
            //            std::size_t jIdx = toIdx(topiIdx, j);
            //            std::size_t jprevIdx = toIdx(topiIdx, j - 1);
            //
            //            triangleIndices.push_back(topIdx);
            //            triangleIndices.push_back(jprevIdx);
            //            triangleIndices.push_back(jIdx);

            //            jIdx = toIdx(bottomiIdx, j);
            //            jprevIdx = toIdx(bottomiIdx, j - 1);
            //
            //            triangleIndices.push_back(jIdx);
            //            triangleIndices.push_back(jprevIdx);
            //            triangleIndices.push_back(bottomIdx);
        }

        // Sphere body triangles
        for (std::size_t i{1}; i < m_polarCount - 1; ++i)
        {
            const std::size_t iprev = i - 1;
            for (std::size_t j{1}; j < m_azimuthCount; ++j)
            {
                const std::size_t jprev = j - 1;

                const std::size_t iprevj = toIdx(iprev, j);
                const std::size_t ijprev = toIdx(i, jprev);

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

template <typename T>
[[deprecated]] Core::TVector<Triangle<T, 3>>
calcSphereTriangles(const Sphere<T>& sphere)
{
    constexpr T CHORD_RADIUS_RATIO = T(0.1);
    const T deltaAngle = std::asin(T(0.5) * CHORD_RADIUS_RATIO) * T(2);

    std::size_t thetaSteps = Core::PI / deltaAngle;
    std::size_t phiSteps = Core::PI_2 / deltaAngle + 1;

    Core::TVector<Triangle<T, 3>> triangles;

    const LinAl::Vec3<T> origin = sphere.getOrigin();
    const T radius = sphere.getRadius();

    Core::TVector<LinAl::Vec3Vector<T>> circles;

    for (std::size_t j{1}; j < thetaSteps; ++j)
    {
        LinAl::Vec3Vector<T> circle;

        const T theta = j * deltaAngle;
        const T sinTheta = std::sin(theta);
        const T cosTheta = std::cos(theta);

        for (std::size_t j{0}; j < phiSteps; ++j)
        {
            const T phi = j * deltaAngle;
            const T sinPhi = std::sin(phi);
            const T cosPhi = std::cos(phi);
            circle.push_back(
                LinAl::Vec3<T>{origin[0] + radius * sinTheta * cosPhi,
                               origin[1] + radius * sinTheta * sinPhi,
                               origin[2] + radius * cosTheta});
        }
        circles.push_back(circle);
    }

    for (std::size_t i{0}; i < circles.size(); i++)
    {
        const std::size_t circlePointsSize = circles[i].size();
        const LinAl::Vec3Vector<T>& circlePoints = circles[i];

        if (i == 0)
        {
            const LinAl::Vec3<T> northPole =
                sphere.getOrigin() + LinAl::Vec3<T>{0, 0, 1} * radius;

            for (std::size_t j{0}; j < circlePointsSize - 1; ++j)
            {
                triangles.push_back(Triangle<T, 3>(northPole,
                                                   circlePoints[j],
                                                   circlePoints[j + 1]));
            }
            triangles.push_back(
                Triangle<T, 3>(northPole,
                               circlePoints[circlePointsSize - 1],
                               circlePoints[0]));
        }
        else
        {
            const LinAl::Vec3Vector<T>& prevCirclePoints = circles[i - 1];

            for (std::size_t j{0}; j < circlePointsSize - 1; ++j)
            {
                triangles.push_back(Triangle<T, 3>(prevCirclePoints[j],
                                                   circlePoints[j],
                                                   circlePoints[j + 1]));
                triangles.push_back(Triangle<T, 3>(circlePoints[j + 1],
                                                   prevCirclePoints[j + 1],
                                                   prevCirclePoints[j]));
            }

            triangles.push_back(
                Triangle<T, 3>(prevCirclePoints[circlePointsSize - 1],
                               circlePoints[circlePointsSize - 1],
                               circlePoints[0]));
            triangles.push_back(
                Triangle<T, 3>(circlePoints[0],
                               prevCirclePoints[0],
                               prevCirclePoints[circlePointsSize - 1]));

            if (i == circles.size() - 1)
            {
                const LinAl::Vec3<T> southPole =
                    sphere.getOrigin() - LinAl::Vec3<T>{0, 0, 1} * radius;

                for (std::size_t j{0}; j < circlePointsSize - 1; ++j)
                {
                    triangles.push_back(Triangle<T, 3>(southPole,
                                                       circlePoints[j + 1],
                                                       circlePoints[j]));
                }
                triangles.push_back(
                    Triangle<T, 3>(southPole,
                                   circlePoints[0],
                                   circlePoints[circlePointsSize - 1]));
            }
        }
    }

    return triangles;
}
} // namespace Geometry

#endif // GEOMETRY_SPHEREMESHBUILDER_HPP
