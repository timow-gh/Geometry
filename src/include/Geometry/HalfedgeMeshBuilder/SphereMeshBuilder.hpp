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
    std::size_t m_thetaCount{10};
    std::size_t m_phiCount{20};
    std::optional<Sphere<T>> m_sphere;

  public:
    SphereMeshBuilder& setThetaCount(std::size_t thetaCount)
    {
        m_thetaCount = thetaCount;
        return *this;
    }

    SphereMeshBuilder& setTPhiCount(std::size_t phiCount)
    {
        m_phiCount = phiCount;
        return *this;
    }

    SphereMeshBuilder& setSphere(const Sphere<T>& sphere)
    {
        m_sphere = sphere;
        return *this;
    }

    std::unique_ptr<HalfedgeMesh<T>> build()
    {
        return nullptr;
        // TODO sphere halfedge mesh
        //        if (!m_sphere)
        //            return nullptr;
        //
        //        auto heMesh =
        //        std::make_unique<HalfedgeMesh<T>>(xg::newGuid()); auto&
        //        meshPoints = heMesh->meshPoints;
        //        meshPoints.setPoints(calcSpherePoints(*m_sphere));
        //
        //        std::size_t pointsSize = meshPoints.size();
        //        for (std::size_t i{0}; i < pointsSize; ++i)
        //            heMesh->m_vertices.emplace_back(i, heMesh.get());
        //
        //        auto& vertices = heMesh->m_vertices;
        //        auto& halfedges = heMesh->m_halfedges;
        //        for (std::size_t i{0}; i < pointsSize; ++i)
        //        {
        //            halfedges.emplace_back(vertices[i].getIndex(),
        //            heMesh.get()); if (i % 2 == 0)
        //            {
        //                asdf
        //            }
        //        }
        //
        //        return heMesh;
    }

  private:
    LinAl::Vec3Vector<T> calcSpherePoints(const Sphere<T>& sphere)
    {
        LinAl::Vec3Vector<T> points;

        T thetaSteps = 2.0 * Core::PI / static_cast<double_t>(m_thetaCount);
        T phiSteps = 2.0 * Core::PI / static_cast<double_t>(m_thetaCount);

        T thetaAngle{0};
        T phiAngle{0};

        const T radius = sphere.getRadius();
        for (std::size_t i{0}; i <= m_thetaCount; ++i)
        {
            thetaAngle = Core::PI / 2 - i * thetaSteps;
            T z = radius * std::cos(thetaAngle);
            T xyPlaneProj = radius * std::sin(thetaAngle);

            for (std::size_t j{0}; j <= m_phiCount; ++j)
            {
                phiAngle = j * phiSteps;

                T x = xyPlaneProj * std::cos(phiAngle);
                T y = xyPlaneProj * std::sin(phiAngle);

                points.push_back(LinAl::Vec3<T>{x, y, z});
            }
        }

        return points;
    }

    Core::TVector<T> calcSphereTriangleIndices() {}
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
