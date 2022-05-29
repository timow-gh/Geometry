#ifndef GEOMETRY_CONEMESHBUILDER_HPP
#define GEOMETRY_CONEMESHBUILDER_HPP

#include <Core/Math/Constants.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CirclePoints.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <optional>

namespace Geometry
{
template <typename TFloatType, typename TIndexType = std::size_t>
class ConeMeshBuilder : public MeshBuilderBase<TFloatType, TIndexType, ConeMeshBuilder<TFloatType, TIndexType>> {
    std::optional<Cone<TFloatType>> m_cone;
    std::size_t m_azimuthCount{20};

  public:
    ConeMeshBuilder() = default;
    ConeMeshBuilder& setCone(const Cone<TFloatType>& cone)
    {
        m_cone = cone;
        return *this;
    }

    ConeMeshBuilder& setAzimuthCount(std::size_t azimuthCount)
    {
        m_azimuthCount = azimuthCount;
        return *this;
    }

    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<TFloatType, TIndexType>> build()
    {
        if (!m_cone)
            return nullptr;

        const auto coneSeg = m_cone->getSegment();

        LinAl::HMatrixd hTrafo = LinAl::rotationAlign(LinAl::Z_HVECD, LinAl::vec3ToHVec(coneSeg.direction()));
        LinAl::setTranslation(hTrafo, coneSeg.getSource());
        MeshBuilderBase<TFloatType, TIndexType, ConeMeshBuilder<TFloatType, TIndexType>>::setTransformation(hTrafo);

        auto conePoints = calcConePoints(*m_cone);
        auto coneTriangleIndices = calcConeTriangleIndices(conePoints);
        return MeshBuilderBase<TFloatType, TIndexType, ConeMeshBuilder<TFloatType, TIndexType>>::buildTriangleHeMesh(
            conePoints,
            coneTriangleIndices);
    }

  private:
    LinAl::Vec3Vector<TFloatType> calcConePoints(const Geometry::Cone<TFloatType>& cone) const
    {
        LinAl::Vec3Vector<TFloatType> points;
        calcCirclePoints(points, cone.getRadius(), m_azimuthCount);

        points.push_back(LinAl::Vec3<TFloatType>{0, 0, 0});
        points.push_back(LinAl::Vec3<TFloatType>{0, 0, cone.getSegment().length()});

        return points;
    }
    Core::TVector<uint32_t> calcConeTriangleIndices(const LinAl::Vec3Vector<TFloatType>& conePoints) const
    {
        Core::TVector<uint32_t> indices;
        // TODO (Safe conversion DebugAssert) Does spherePoints.size() fit into uint32_t
        uint32_t size = static_cast<uint32_t>(conePoints.size());
        uint32_t topIdx = size - 1;
        uint32_t bottomIdx = size - 2;
        for (uint32_t i{1}; i < size; ++i)
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
