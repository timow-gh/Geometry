#ifndef GLFWTESTAPP_CUBE_H
#define GLFWTESTAPP_CUBE_H

#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Cuboid {
  public:
    CORE_CONSTEXPR Cuboid(const LinAl::Vec3<T>& origin, const LinAl::Vec3Array<T, 3>& sideVectors)
        : m_origin(origin), m_sideVectors(sideVectors)
    {
    }

    CORE_CONSTEXPR Cuboid(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& diagonal)
        : m_origin(origin)
    {
        LinAl::Vec3Array<T, 3> unitVectors = {
            LinAl::X_VEC3D,
            LinAl::Y_VEC3D,
            LinAl::Z_VEC3D,
        };
        for (std::size_t i = 0; i < 3; ++i)
            m_sideVectors[i] = LinAl::projection(diagonal, unitVectors[i]);
    }

    CORE_CONSTEXPR explicit Cuboid(const LinAl::Vec3<T>& diagonal)
        : Cuboid(LinAl::Vec3<T>{0, 0, 0}, diagonal)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& origin() const
    {
        return m_origin;
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3Array<T, 3>& sideVectors() const
    {
        return m_sideVectors;
    }

    CORE_CONSTEXPR bool operator==(const Cuboid& rhs) const
    {
        return m_origin == rhs.m_origin && m_sideVectors == rhs.m_sideVectors;
    }

    CORE_CONSTEXPR bool operator!=(const Cuboid& rhs) const
    {
        return !(rhs == *this);
    }

  private:
    LinAl::Vec3<T> m_origin;
    LinAl::Vec3Array<T, 3> m_sideVectors;
};

template <typename T>
Cuboid<T> transformation(const Cuboid<T>& cuboid, const LinAl::HMatrix<T>& trafo)
{
    LinAl::HVec<T> origin = trafo * LinAl::vec3ToHVec(cuboid.getOrigin());
    return Cuboid<T>{LinAl::hVecToVec3(origin), cuboid.getSideVectors()};
}

template <typename T>
CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3Array<T, 8> calcCuboidVertices(const Cuboid<T>& cuboid)
{
    LinAl::Vec3<T> origin = cuboid.origin();
    const auto& sideVecs = cuboid.sideVectors();
    LinAl::Vec3Array<T, 8> vertices;
    const auto bottomX = sideVecs[0];
    const auto bottomY = sideVecs[1];
    const auto bottomZ = sideVecs[2];
    vertices[0] = LinAl::Vec3<T>{origin};
    vertices[1] = LinAl::Vec3<T>{origin + bottomX};
    vertices[2] = LinAl::Vec3<T>{origin + bottomX + bottomY};
    vertices[3] = LinAl::Vec3<T>{origin + bottomY};
    vertices[4] = LinAl::Vec3<T>{origin + bottomZ};
    vertices[5] = LinAl::Vec3<T>{origin + bottomX + bottomZ};
    vertices[6] = LinAl::Vec3<T>{origin + bottomX + bottomY + bottomZ};
    vertices[7] = LinAl::Vec3<T>{origin + bottomY + bottomZ};
    return vertices;
}

} // namespace Geometry

#endif // GLFWTESTAPP_CUBE_H
