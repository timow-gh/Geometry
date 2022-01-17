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

} // namespace Geometry

#endif // GLFWTESTAPP_CUBE_H
