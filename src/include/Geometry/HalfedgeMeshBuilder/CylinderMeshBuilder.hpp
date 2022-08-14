#ifndef FILAPP_CYLINDERMESHBUILDER_HPP
#define FILAPP_CYLINDERMESHBUILDER_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CirclePoints.hpp>
#include <Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp>
#include <Geometry/Segment.hpp>
#include <optional>

namespace Geometry
{
template <typename TFloatType, typename TIndexType = std::size_t>
class CylinderMeshBuilder : public MeshBuilderBase<TFloatType, TIndexType, CylinderMeshBuilder<TFloatType, TIndexType>> {
    std::optional<Cylinder<TFloatType>> m_cylinder;
    std::size_t m_azimuthCount{20};

  public:
    CylinderMeshBuilder() = default;

    CylinderMeshBuilder& setCylinder(const Cylinder<TFloatType>& clyinder)
    {
        m_cylinder = clyinder;
        return *this;
    }

    CylinderMeshBuilder& setAzimuthCount(std::size_t azimuthCount)
    {
        m_azimuthCount = azimuthCount;
        return *this;
    }

    CORE_NODISCARD std::unique_ptr<HalfedgeMesh<TFloatType, TIndexType>> build()
    {
        if (!m_cylinder)
            return nullptr;

        const auto cylinderSeg = m_cylinder->getSegment();

        LinAl::HMatrixd hTrafo = LinAl::hMatRotationAlign(LinAl::Z_HVECD, LinAl::vec3ToHVec(cylinderSeg.direction()));
        LinAl::setTranslation(hTrafo, cylinderSeg.getSource());
        MeshBuilderBase<TFloatType, TIndexType, CylinderMeshBuilder<TFloatType, TIndexType>>::setTransformation(hTrafo);

        LinAl::Vec3Vector<TFloatType> cylPoints = calcCylinderPoints();
        const auto cylinderTriangleIndices = calcCylinderTriangleIndices(cylPoints);
        return MeshBuilderBase<TFloatType, TIndexType, CylinderMeshBuilder<TFloatType, TIndexType>>::buildTriangleHeMesh(
            cylPoints,
            cylinderTriangleIndices);
    }

  private:
    LinAl::Vec3Vector<TFloatType> calcCylinderPoints() const
    {
        LinAl::Vec3Vector<TFloatType> points;
        points.reserve(2 * m_azimuthCount + 2);

        const Segment3<TFloatType>& segment = m_cylinder->getSegment();
        calcCirclePoints(points, m_cylinder->getRadius(), m_azimuthCount);

        std::size_t circlePointsSize = points.size();
        double_t segLength = m_cylinder->getSegment().length();
        for (std::size_t i{0}; i < circlePointsSize; ++i)
        {
            points.push_back(points[i]);
            points.back()[2] += segLength;
        }

        points.push_back(LinAl::Vec3<TFloatType>{0, 0, 0});
        points.push_back(LinAl::Vec3<TFloatType>{0, 0, segLength});

        return points;
    }

    void calcCircleBufferIndices(Core::TVector<uint32_t>& indices,
                                 const std::size_t midPointIdx,
                                 const std::size_t circleStartIdx,
                                 const std::size_t circleEndIdx) const
    {
        for (std::size_t i{circleStartIdx + 1}; i < circleEndIdx; i++)
        {
            indices.push_back(midPointIdx);
            indices.push_back(i - 1);
            indices.push_back(i);
        }
        indices.push_back(midPointIdx);
        indices.push_back(circleStartIdx);
        indices.push_back(circleEndIdx - 1);
    }

    Core::TVector<uint32_t> calcCylinderTriangleIndices(const LinAl::Vec3Vector<TFloatType>& cylinderPoints) const
    {
        // Bottom circle
        Core::TVector<uint32_t> indices;
        const std::size_t cylPointsSize = cylinderPoints.size();

        const std::size_t bottomMidPointIdx = cylPointsSize - 2;
        const std::size_t bottomCircleStartIdx = 0;
        const std::size_t bottomCircleEndIdx = 0 + m_azimuthCount;
        calcCircleBufferIndices(indices, bottomMidPointIdx, bottomCircleStartIdx, bottomCircleEndIdx);

        // Outer surface
        for (std::size_t i{1}; i < m_azimuthCount; ++i)
        {
            const std::size_t topIdx = i + m_azimuthCount;
            const std::size_t topIdxMOne = i + m_azimuthCount - 1;
            indices.push_back(i);
            indices.push_back(topIdx);
            indices.push_back(topIdxMOne);

            indices.push_back(i);
            indices.push_back(topIdxMOne);
            indices.push_back(i - 1);
        }
        const std::size_t topIdxStart = m_azimuthCount;
        const std::size_t topIdxEnd = 2 * m_azimuthCount - 1;
        indices.push_back(topIdxEnd);
        indices.push_back(topIdxStart - 1);
        indices.push_back(0);
        indices.push_back(topIdxEnd);
        indices.push_back(0);
        indices.push_back(topIdxStart);

        // Top circle
        const std::size_t topMidPointIdx = cylPointsSize - 1;
        const std::size_t topCircleStartIdx = m_azimuthCount;
        const std::size_t topCircleEndIdx = 2 * m_azimuthCount;
        calcCircleBufferIndices(indices, topMidPointIdx, topCircleStartIdx, topCircleEndIdx);

        return indices;
    }
};
} // namespace Geometry

#endif // FILAPP_CYLINDERMESHBUILDER_HPP
