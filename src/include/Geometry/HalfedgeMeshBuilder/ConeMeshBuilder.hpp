#ifndef GEOMETRY_CONEMESHBUILDER_HPP
#define GEOMETRY_CONEMESHBUILDER_HPP

#include <Core/Math/Constants.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <optional>

namespace Geometry
{
template <typename T>
class ConeMeshBuilder
    : public MeshBuilderBase<T>
{
    std::optional<Cone<T>> m_cone;
    std::size_t m_azimuthCount{20};

  public:
    ConeMeshBuilder() = default;
    ConeMeshBuilder& setCone(const Cone<T>& cone)
    {
        m_cone = cone;
        return *this;
    }

    ConeMeshBuilder& setAzimuth(std::size_t azimuthCount)
    {
        m_azimuthCount = azimuthCount;
        return *this;
    }

    [[nodiscard]] std::unique_ptr<HalfedgeMesh<T>> build()
    {
        if (!m_cone)
            return nullptr;

        auto conePoints = calcConePoints(*m_cone);
        auto coneTriangleIndices = calcConeTriangleIndices(conePoints);
        return MeshBuilderBase<T>::buildTriangleHeMesh(conePoints,
                                                       coneTriangleIndices);
    }

  private:
    LinAl::Vec3Vector<T> calcConePoints(const Geometry::Cone<T>& cone)
    {
        T azimuthStep = 2.0 * Core::PI / static_cast<double_t>(m_azimuthCount);
        T circleRadius = cone.getRadius();

        LinAl::Vec3Vector<T> points;
        for (std::size_t i{0}; i < m_azimuthCount; ++i)
        {
            T azimuthAngle = i * azimuthStep;
            points.push_back(LinAl::Vec3<T>{std::cos(azimuthAngle),
                                            std::sin(azimuthAngle),
                                            0.0});
        }

        points.push_back(LinAl::Vec3<T>{0, 0, 0});
        points.push_back(LinAl::Vec3<T>{0, 0, cone.getSegment().length()});

        return points;
    }
    Core::TVector<uint32_t>
    calcConeTriangleIndices(const LinAl::Vec3Vector<T>& conePoints)
    {
        Core::TVector<uint32_t> indices;
        std::size_t size = conePoints.size();
        std::size_t topIdx = size - 1;
        std::size_t bottomIdx = size - 2;
        for (std::size_t i{1}; i < size; ++i)
        {
            // slant surface
            indices.push_back(topIdx);
            indices.push_back(i - 1);
            indices.push_back(i);
            // base
            indices.push_back(bottomIdx);
            indices.push_back(i - 1);
            indices.push_back(i);
        }
        // slant surface, connecting triangle
        indices.push_back(topIdx);
        indices.push_back(size - 3);
        indices.push_back(0);
        // base, connecting triangle
        indices.push_back(bottomIdx);
        indices.push_back(size - 3);
        indices.push_back(0);

        return indices;
    }
};
} // namespace Geometry

#endif // GEOMETRY_CONEMESHBUILDER_HPP
