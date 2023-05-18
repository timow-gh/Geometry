#ifndef GLFWTESTAPP_CUBE_H
#define GLFWTESTAPP_CUBE_H

#include <Geometry/Utils/Compiler.hpp>
#include <linal/Containers.hpp>
#include <linal/Vec3.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{

template <typename T>
class Cuboid {
public:
  GEO_CONSTEXPR Cuboid(const linal::Vec3<T>& origin, const linal::Vec3Array<T, 3>& sideVectors)
      : m_origin(origin)
      , m_sideVectors(sideVectors)
  {
  }

  GEO_CONSTEXPR Cuboid(const linal::Vec3<T>& origin, const linal::Vec3<T>& diagonal)
      : m_origin(origin)
  {
    linal::Vec3Array<T, 3> unitVectors = {
        linal::X_VEC3D,
        linal::Y_VEC3D,
        linal::Z_VEC3D,
    };
    for (std::size_t i = 0; i < 3; ++i)
      m_sideVectors[i] = linal::projection(diagonal, unitVectors[i]);
  }

  GEO_CONSTEXPR explicit Cuboid(const linal::Vec3<T>& diagonal)
      : Cuboid(linal::Vec3<T>{0, 0, 0}, diagonal)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec3Array<T, 3>& getSideVectors() const { return m_sideVectors; }

  GEO_CONSTEXPR bool operator==(const Cuboid& rhs) const { return m_origin == rhs.m_origin && m_sideVectors == rhs.m_sideVectors; }
  GEO_CONSTEXPR bool operator!=(const Cuboid& rhs) const { return !(rhs == *this); }

private:
  linal::Vec3<T> m_origin;
  linal::Vec3Array<T, 3> m_sideVectors;
};

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR linal::Vec3Array<T, 8> calcCuboidVertices(const Cuboid<T>& cuboid)
{
  linal::Vec3<T> origin = cuboid.getOrigin();
  const auto& sideVecs = cuboid.getSideVectors();
  linal::Vec3Array<T, 8> vertices;
  const auto bottomX = sideVecs[0];
  const auto bottomY = sideVecs[1];
  const auto bottomZ = sideVecs[2];
  vertices[0] = linal::Vec3<T>{origin};
  vertices[1] = linal::Vec3<T>{origin + bottomX};
  vertices[2] = linal::Vec3<T>{origin + bottomX + bottomY};
  vertices[3] = linal::Vec3<T>{origin + bottomY};
  vertices[4] = linal::Vec3<T>{origin + bottomZ};
  vertices[5] = linal::Vec3<T>{origin + bottomX + bottomZ};
  vertices[6] = linal::Vec3<T>{origin + bottomX + bottomY + bottomZ};
  vertices[7] = linal::Vec3<T>{origin + bottomY + bottomZ};
  return vertices;
}

} // namespace Geometry

#endif // GLFWTESTAPP_CUBE_H
