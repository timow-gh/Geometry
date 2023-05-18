#ifndef GLFWTESTAPP_LCS_H
#define GLFWTESTAPP_LCS_H

#include <Geometry/Utils/Compiler.hpp>
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
  GEO_CONSTEXPR Lcs()
      : m_origin(LinAl::Vec3<T>{0})
      , m_coordVec{LinAl::Vec3<T>{1, 0, 0}, LinAl::Vec3<T>{0, 1, 0}, LinAl::Vec3<T>{0, 0, 1}}
  {
  }

  GEO_CONSTEXPR
  Lcs(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& x, const LinAl::Vec3<T>& y)
      : m_origin(origin)
  {
    m_coordVec[0] = x;
    m_coordVec[1] = y;
    m_coordVec[2] = cross(x, y);
    normalizeCoordVectors();
  }

  GEO_CONSTEXPR Lcs(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& x, const LinAl::Vec3<T>& y, const LinAl::Vec3<T>& z)
      : m_origin(origin)
      , m_coordVec({x, y, z})
  {
    normalizeCoordVectors();
  }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const { return m_origin; }
  GEO_CONSTEXPR void setOrigin(const LinAl::Vec3<T>& origin) { m_origin = origin; }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3Array<T, 3>& getCoordVectors() const { return m_coordVec; }
  GEO_CONSTEXPR void setCoordVectors(const LinAl::Vec3Array<T, 3>& coordVectors) { m_coordVec = coordVectors; }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getXVector() const { return m_coordVec[0]; }
  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getYVector() const { return m_coordVec[1]; }
  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getZVector() const { return m_coordVec[2]; }

  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3Array<T, 3> projectVector(const LinAl::Vec3<T>& vector) const
  {
    LinAl::Vec3Array<T, 3> result;
    for (std::size_t i = 0; i < m_coordVec.size(); ++i)
      result[i] = projection(vector, m_coordVec[i]);
    return result;
  }

  GEO_CONSTEXPR bool operator==(const Lcs& rhs) const { return m_origin == rhs.m_origin && m_coordVec == rhs.m_coordVec; }
  GEO_CONSTEXPR bool operator!=(const Lcs& rhs) const { return !(rhs == *this); }

private:
  GEO_CONSTEXPR void normalizeCoordVectors()
  {
    for (auto& vec: m_coordVec)
      vec = normalize(vec);
  }
};

} // namespace Geometry

#endif // GLFWTESTAPP_LCS_H
