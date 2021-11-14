#ifndef GLFWTESTAPP_LCS_H
#define GLFWTESTAPP_LCS_H

#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{

template <typename T>
class Lcs
{
  private:
    LinAl::Vec3<T> m_origin;
    //! x,y,z vectors
    LinAl::Vec3Array<T, 3> m_coordVec;

  public:
    constexpr Lcs();
    constexpr Lcs(const LinAl::Vec3<T>& origin,
                  const LinAl::Vec3<T>& x,
                  const LinAl::Vec3<T>& y);
    constexpr Lcs(const LinAl::Vec3<T>& origin,
                  const LinAl::Vec3<T>& x,
                  const LinAl::Vec3<T>& y,
                  const LinAl::Vec3<T>& z);

    bool operator==(const Lcs& rhs) const;
    bool operator!=(const Lcs& rhs) const;

    const LinAl::Vec3<T>& getOrigin() const;
    void setOrigin(const LinAl::Vec3<T>& origin);

    const LinAl::Vec3Array<T, 3>& getCoordVectors() const;
    void setCoordVectors(const LinAl::Vec3Array<T, 3>& coordVectors);

    const LinAl::Vec3<T>& getXVector() const;
    const LinAl::Vec3<T>& getYVector() const;
    const LinAl::Vec3<T>& getZVector() const;

    LinAl::Vec3Array<T, 3> projectVector(const LinAl::Vec3<T>& vector) const;

  private:
    void normalizeCoordVectors();
};

template <typename T>
constexpr Lcs<T>::Lcs()
    : m_origin(LinAl::Vec3<T>{0})
    , m_coordVec{LinAl::Vec3<T>{1, 0, 0},
                 LinAl::Vec3<T>{0, 1, 0},
                 LinAl::Vec3<T>{0, 0, 1}}
{
}

template <typename T>
constexpr Lcs<T>::Lcs(const LinAl::Vec3<T>& origin,
                      const LinAl::Vec3<T>& x,
                      const LinAl::Vec3<T>& y)
    : m_origin(origin)
{
    m_coordVec[0] = x;
    m_coordVec[1] = y;
    m_coordVec[2] = cross(x, y);
    normalizeCoordVectors();
}

template <typename T>
constexpr Lcs<T>::Lcs(const LinAl::Vec3<T>& origin,
                      const LinAl::Vec3<T>& x,
                      const LinAl::Vec3<T>& y,
                      const LinAl::Vec3<T>& z)
    : m_origin(origin), m_coordVec({x, y, z})
{
    normalizeCoordVectors();
}

template <typename T>
bool Lcs<T>::operator==(const Lcs& rhs) const
{
    return std::tie(m_origin, m_coordVec) ==
           std::tie(rhs.m_origin, rhs.m_coordVec);
}

template <typename T>
bool Lcs<T>::operator!=(const Lcs& rhs) const
{
    return !(rhs == *this);
}

template <typename T>
const LinAl::Vec3<T>& Lcs<T>::getOrigin() const
{
    return m_origin;
}

template <typename T>
void Lcs<T>::setOrigin(const LinAl::Vec3<T>& origin)
{
    m_origin = origin;
}

template <typename T>
const LinAl::Vec3Array<T, 3>& Lcs<T>::getCoordVectors() const
{
    return m_coordVec;
}

template <typename T>
void Lcs<T>::setCoordVectors(const LinAl::Vec3Array<T, 3>& coordVectors)
{
    m_coordVec = coordVectors;
}

template <typename T>
LinAl::Vec3Array<T, 3> Lcs<T>::projectVector(const LinAl::Vec3<T>& vector) const
{
    LinAl::Vec3Array<T, 3> result;
    for (std::size_t i = 0; i < m_coordVec.size(); ++i)
        result[i] = projection(vector, m_coordVec[i]);
    return result;
}

template <typename T>
const LinAl::Vec3<T>& Lcs<T>::getXVector() const
{
    return m_coordVec[0];
}
template <typename T>
const LinAl::Vec3<T>& Lcs<T>::getYVector() const
{
    return m_coordVec[1];
}
template <typename T>
const LinAl::Vec3<T>& Lcs<T>::getZVector() const
{
    return m_coordVec[2];
}

template <typename T>
void Lcs<T>::normalizeCoordVectors()
{
    for (auto& vec: m_coordVec)
        vec = normalize(vec);
}

} // namespace Geometry

#endif // GLFWTESTAPP_LCS_H
