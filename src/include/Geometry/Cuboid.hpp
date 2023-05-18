#ifndef GLFWTESTAPP_CUBE_H
#define GLFWTESTAPP_CUBE_H

#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Cuboid {
public:
  GEO_CONSTEXPR Cuboid(const LinAl::Vec3<T>& origin, const LinAl::Vec3Array<T, 3>& sideVectors)
      : m_origin(origin)
      , m_sideVectors(sideVectors)
  {
  }

  GEO_CONSTEXPR Cuboid(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& diagonal)
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

  GEO_CONSTEXPR explicit Cuboid(const LinAl::Vec3<T>& diagonal)
      : Cuboid(LinAl::Vec3<T>{0, 0, 0}, diagonal)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3Array<T, 3>& getSideVectors() const { return m_sideVectors; }

  GEO_CONSTEXPR bool operator==(const Cuboid& rhs) const { return m_origin == rhs.m_origin && m_sideVectors == rhs.m_sideVectors; }
  GEO_CONSTEXPR bool operator!=(const Cuboid& rhs) const { return !(rhs == *this); }

private:
  LinAl::Vec3<T> m_origin;
  LinAl::Vec3Array<T, 3> m_sideVectors;
};

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3Array<T, 8> calcCuboidVertices(const Cuboid<T>& cuboid)
{
  LinAl::Vec3<T> origin = cuboid.getOrigin();
  const auto& sideVecs = cuboid.getSideVectors();
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
