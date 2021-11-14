#ifndef GLFWTESTAPP_CUBE_H
#define GLFWTESTAPP_CUBE_H

#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{

template <typename T>
class Cuboid
{
  public:
    constexpr Cuboid(const LinAl::Vec3<T>& origin,
                     const LinAl::Vec3Array<T, 3>& sideVectors);
    constexpr Cuboid(const LinAl::Vec3<T>& origin,
                     const LinAl::Vec3<T>& diagonal);
    constexpr explicit Cuboid(const LinAl::Vec3<T>& diagonal);

    [[nodiscard]] const LinAl::Vec3<T>& origin() const;
    [[nodiscard]] const LinAl::Vec3Array<T, 3>& sideVectors() const;

    bool operator==(const Cuboid& rhs) const;
    bool operator!=(const Cuboid& rhs) const;

  private:
    LinAl::Vec3<T> m_origin;
    LinAl::Vec3Array<T, 3> m_sideVectors;
};

template <typename T>
constexpr Cuboid<T>::Cuboid(const LinAl::Vec3<T>& origin,
                            const LinAl::Vec3Array<T, 3>& sideVectors)
    : m_origin(origin), m_sideVectors(sideVectors)
{
}

template <typename T>
constexpr Cuboid<T>::Cuboid(const LinAl::Vec3<T>& origin,
                            const LinAl::Vec3<T>& diagonal)
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

template <typename T>
constexpr Cuboid<T>::Cuboid(const LinAl::Vec3<T>& diagonal)
    : Cuboid(LinAl::Vec3<T>{0, 0, 0}, diagonal)
{
}
template <typename T>
const LinAl::Vec3<T>& Cuboid<T>::origin() const
{
    return m_origin;
}
template <typename T>
const LinAl::Vec3Array<T, 3>& Cuboid<T>::sideVectors() const
{
    return m_sideVectors;
}
template <typename T>
bool Cuboid<T>::operator==(const Cuboid& rhs) const
{
    return m_origin == rhs.m_origin && m_sideVectors == rhs.m_sideVectors;
}
template <typename T>
bool Cuboid<T>::operator!=(const Cuboid& rhs) const
{
    return !(rhs == *this);
}

} // namespace Geometry

#endif // GLFWTESTAPP_CUBE_H
