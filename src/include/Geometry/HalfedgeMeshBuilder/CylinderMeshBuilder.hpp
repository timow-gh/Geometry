#ifndef FILAPP_CYLINDERMESHBUILDER_HPP
#define FILAPP_CYLINDERMESHBUILDER_HPP

#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CirclePoints.hpp>
#include <Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp>
#include <Geometry/Segment.hpp>
#include <optional>

namespace Geometry
{
template <typename T>
    class CylinderMeshBuilder : public MeshBuilderBase < T
    , CylinderMeshBuilder<T>
{
    std::optional<Cylinder<T>> m_cylinder;
    std::size_t m_azimuthCount{20};

  public:
    CylinderMeshBuilder() = default;

    CylinderMeshBuilder& setCylinder(const Cylinder<T>& clyinder)
    {
        m_cylinder = clyinder;
        return *this;
    }

    CylinderMeshBuilder& setAzimuthCount(std::size_t azimuthCount)
    {
        m_azimuthCount = azimuthCount;
        return *this;
    }

    [[nodiscard]] std::unique_ptr<HalfedgeMesh<T>> build()
    {
        if (!m_cylinder)
            return nullptr;

        const auto cylinderSeg = m_cone->getSegment();

        LinAl::HMatrixd hTrafo =
            LinAl::rotationAlign(LinAl::Z_HVECD,
                                 LinAl::vec3ToHVec(cylinderSeg.direction()));
        LinAl::setTranslation(hTrafo, cylinderSeg.getSource());
        MeshBuilderBase<T, CylinderMeshBuilder<T>>::setTransformation(hTrafo);

        LinAl::Vec3Vector<T> cylPoints = calcCylinderPoints();
        const auto cylinderTriangleIndices =
            calcCylinderTriangleIndices(cylinderPoints);
        return MeshBuilderBase<T, CylinderMeshBuilder<T>>::buildTriangleHeMesh(
            cylPoints,
            cylinderTriangleIndices);
    }

  private:
    LinAl::Vec3Vector<T> calcCylinderPoints() const
    {
        LinAl::Vec3Vector<T> points;
        points.reserve(2 * m_azimuthCount + 2);

        const Segment<T>& segment = m_cylinder->getSegment();
        calcCirclePoints(points, cone.getRadius(), m_azimuthCount);

        std::size_t circlePointsSize = points.size();
        double_t segLength = m_cylinder->getSegment().length();
        for (std::size_t i{0}; i < circlePointsSize; ++i)
        {
            points.push_back(points[i]);
            points.back()[2] += segLength;
        }

        points.push_back(segment.getSource());
        points.push_back(segment.getTarget());

        return points;
    }

    void calcCircleBufferIndices(Core::TVector<uint32_t>& indices,
                                 const LinAl::Vec3Vector<T>& cylinderPoints,
                                 const std::size_t midPointIdx,
                                 const std::size_t circleStartIdx,
                                 const std::size_t circleEndIdx) const
    {
        for (std::size_t i{circleStartIdx + 1}; i < circleEndIdx; i++)
        {
            indices.push_back(midPointIdx);
            indices.push_back(cylinderPoints[i - 1]);
            indices.push_back(cylinderPoints[i]);
        }
    }

    Core::TVector<uint32_t> calcCylinderTriangleIndices(
        const LinAl::Vec3Vector<T>& cylinderPoints) const
    {
        Core::TVector<uint32_t> indices;
        const std::size_t cylPointsSize = cylinderPoints.size();

        const std::size_t bottomMidPointIdx = cylinderPoints[cylPointsSize - 2];
        const std::size_t bottomCircleStartIdx = 0;
        const std::size_t bottomCircleEndIdx = 0 + m_azimuthCount;
        calcCircleBufferIndices(indices,
                                cylinderPoints,
                                bottomMidPointIdx,
                                bottomCircleStartIdx,
                                bottomCircleEndIdx);

        for (std::size_t i{1}; i < m_azimuthCount; ++i)
        {
            std::size_t topIdx = i + m_azimuthCount;
            std::size_t topIdxMOne = i + m_azimuthCount - 1;
            indices.push_back(i);
            indices.push_back(topIdx);
            indices.push_back(topIdxMOne);

            indices.push_back(i);
            indices.push_back(topIdxMOne);
            indices.push_back(i - 1);
        }

        const std::size_t topMidPointIdx = cylinderPoints[cylPointsSize - 1];
        const std::size_t topCircleStartIdx = m_azimuthCount;
        const std::size_t topCircleEndIdx = 2 * m_azimuthCount;
        calcCircleBufferIndices(indices,
                                cylinderPoints,
                                topMidPointIdx,
                                topCircleStartIdx,
                                topCircleEndIdx);
    }
}
};
} // namespace Geometry

#endif // FILAPP_CYLINDERMESHBUILDER_HPP
