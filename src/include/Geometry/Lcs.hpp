#ifndef GLFWTESTAPP_LCS_H
#define GLFWTESTAPP_LCS_H

#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Lcs {
  private:
    LinAl::Vec3<T> m_origin;
    //! x,y,z vectors
    LinAl::Vec3Array<T, 3> m_coordVec;

  public:
    CORE_CONSTEXPR Lcs();
    CORE_CONSTEXPR
    Lcs(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& x, const LinAl::Vec3<T>& y);
    CORE_CONSTEXPR Lcs(const LinAl::Vec3<T>& origin,
                       const LinAl::Vec3<T>& x,
                       const LinAl::Vec3<T>& y,
                       const LinAl::Vec3<T>& z);

    CORE_CONSTEXPR bool operator==(const Lcs& rhs) const;
    CORE_CONSTEXPR bool operator!=(const Lcs& rhs) const;

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const;
    CORE_CONSTEXPR void setOrigin(const LinAl::Vec3<T>& origin);

    CORE_CONSTEXPR const LinAl::Vec3Array<T, 3>& getCoordVectors() const;
    CORE_CONSTEXPR void setCoordVectors(const LinAl::Vec3Array<T, 3>& coordVectors);

    CORE_CONSTEXPR const LinAl::Vec3<T>& getXVector() const;
    CORE_CONSTEXPR const LinAl::Vec3<T>& getYVector() const;
    CORE_CONSTEXPR const LinAl::Vec3<T>& getZVector() const;

    CORE_CONSTEXPR LinAl::Vec3Array<T, 3> projectVector(const LinAl::Vec3<T>& vector) const;

  private:
    CORE_CONSTEXPR void normalizeCoordVectors();
};

template <typename T>
CORE_CONSTEXPR Lcs<T>::Lcs()
    : m_origin(LinAl::Vec3<T>{0})
    , m_coordVec{LinAl::Vec3<T>{1, 0, 0}, LinAl::Vec3<T>{0, 1, 0}, LinAl::Vec3<T>{0, 0, 1}}
{
}

template <typename T>
CORE_CONSTEXPR
Lcs<T>::Lcs(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& x, const LinAl::Vec3<T>& y)
    : m_origin(origin)
{
    m_coordVec[0] = x;
    m_coordVec[1] = y;
    m_coordVec[2] = cross(x, y);
    normalizeCoordVectors();
}

template <typename T>
CORE_CONSTEXPR Lcs<T>::Lcs(const LinAl::Vec3<T>& origin,
                           const LinAl::Vec3<T>& x,
                           const LinAl::Vec3<T>& y,
                           const LinAl::Vec3<T>& z)
    : m_origin(origin), m_coordVec({x, y, z})
{
    normalizeCoordVectors();
}

template <typename T>
CORE_CONSTEXPR bool Lcs<T>::operator==(const Lcs& rhs) const
{
    return std::tie(m_origin, m_coordVec) == std::tie(rhs.m_origin, rhs.m_coordVec);
}

template <typename T>
CORE_CONSTEXPR bool Lcs<T>::operator!=(const Lcs& rhs) const
{
    return !(rhs == *this);
}

template <typename T>
CORE_CONSTEXPR const LinAl::Vec3<T>& Lcs<T>::getOrigin() const
{
    return m_origin;
}

template <typename T>
CORE_CONSTEXPR void Lcs<T>::setOrigin(const LinAl::Vec3<T>& origin)
{
    m_origin = origin;
}

template <typename T>
CORE_CONSTEXPR const LinAl::Vec3Array<T, 3>& Lcs<T>::getCoordVectors() const
{
    return m_coordVec;
}

template <typename T>
CORE_CONSTEXPR void Lcs<T>::setCoordVectors(const LinAl::Vec3Array<T, 3>& coordVectors)
{
    m_coordVec = coordVectors;
}

template <typename T>
CORE_CONSTEXPR LinAl::Vec3Array<T, 3> Lcs<T>::projectVector(const LinAl::Vec3<T>& vector) const
{
    LinAl::Vec3Array<T, 3> result;
    for (std::size_t i = 0; i < m_coordVec.size(); ++i)
        result[i] = projection(vector, m_coordVec[i]);
    return result;
}

template <typename T>
CORE_CONSTEXPR const LinAl::Vec3<T>& Lcs<T>::getXVector() const
{
    return m_coordVec[0];
}

template <typename T>
CORE_CONSTEXPR const LinAl::Vec3<T>& Lcs<T>::getYVector() const
{
    return m_coordVec[1];
}

template <typename T>
CORE_CONSTEXPR const LinAl::Vec3<T>& Lcs<T>::getZVector() const
{
    return m_coordVec[2];
}

template <typename T>
CORE_CONSTEXPR void Lcs<T>::normalizeCoordVectors()
{
    for (auto& vec: m_coordVec)
        vec = normalize(vec);
}

} // namespace Geometry

#endif // GLFWTESTAPP_LCS_H
