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
    CORE_CONSTEXPR Lcs()
        : m_origin(LinAl::Vec3<T>{0}), m_coordVec{LinAl::Vec3<T>{1, 0, 0}, LinAl::Vec3<T>{0, 1, 0}, LinAl::Vec3<T>{0, 0, 1}}
    {
    }

    CORE_CONSTEXPR
    Lcs(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& x, const LinAl::Vec3<T>& y) : m_origin(origin)
    {
        m_coordVec[0] = x;
        m_coordVec[1] = y;
        m_coordVec[2] = cross(x, y);
        normalizeCoordVectors();
    }

    CORE_CONSTEXPR Lcs(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& x, const LinAl::Vec3<T>& y, const LinAl::Vec3<T>& z)
        : m_origin(origin), m_coordVec({x, y, z})
    {
        normalizeCoordVectors();
    }

    CORE_CONSTEXPR bool operator==(const Lcs& rhs) const
    {
        return std::tie(m_origin, m_coordVec) == std::tie(rhs.m_origin, rhs.m_coordVec);
    }

    CORE_CONSTEXPR bool operator!=(const Lcs& rhs) const { return !(rhs == *this); }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const { return m_origin; }
    CORE_CONSTEXPR void setOrigin(const LinAl::Vec3<T>& origin) { m_origin = origin; }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3Array<T, 3>& getCoordVectors() const { return m_coordVec; }
    CORE_CONSTEXPR void setCoordVectors(const LinAl::Vec3Array<T, 3>& coordVectors) { m_coordVec = coordVectors; }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getXVector() const { return m_coordVec[0]; }
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getYVector() const { return m_coordVec[1]; }
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getZVector() const { return m_coordVec[2]; }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3Array<T, 3> projectVector(const LinAl::Vec3<T>& vector) const
    {
        LinAl::Vec3Array<T, 3> result;
        for (std::size_t i = 0; i < m_coordVec.size(); ++i)
            result[i] = projection(vector, m_coordVec[i]);
        return result;
    }

  private:
    CORE_CONSTEXPR void normalizeCoordVectors()
    {
        for (auto& vec: m_coordVec)
            vec = normalize(vec);
    }
};

} // namespace Geometry

#endif // GLFWTESTAPP_LCS_H
