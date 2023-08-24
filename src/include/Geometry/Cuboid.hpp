#ifndef GEOMETRY_CUBE_H
#define GEOMETRY_CUBE_H

#include "Geometry/Utils/Compiler.hpp"
#include <linal/containers.hpp>
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T>
class Cuboid {
public:
  constexpr Cuboid(linal::vec3<T> origin, const linal::vec3Array<T, 3>& sideVectors) noexcept
      : m_origin(origin)
      , m_sideVectors(sideVectors)
  {
  }

  constexpr Cuboid(linal::vec3<T> origin, const linal::vec3<T>& diagonal) noexcept
      : m_origin(origin)
  {
    linal::vec3Array<T, 3> unitVectors = {
        linal::X_VEC3D,
        linal::Y_VEC3D,
        linal::Z_VEC3D,
    };
    for (std::size_t i = 0; i < 3; ++i)
    {
      m_sideVectors[i] = linal::projection(diagonal, unitVectors[i]);
    }
  }

  constexpr explicit Cuboid(const linal::vec3<T>& diagonal) noexcept
      : Cuboid(linal::vec3<T>{0, 0, 0}, diagonal)
  {
  }

  GEO_NODISCARD constexpr linal::vec3<T> get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr const linal::vec3Array<T, 3>& get_side_vectors() const noexcept { return m_sideVectors; }

  constexpr void set_origin(linal::vec3<T> origin) noexcept { m_origin = origin; }
  constexpr void set_side_vectors(const linal::vec3Array<T, 3>& sideVectors) noexcept { m_sideVectors = sideVectors; }

  constexpr bool operator==(const Cuboid& rhs) const noexcept
  {
    return linal::is_equal(m_origin, rhs.m_origin) && linal::is_equal(m_sideVectors[0], rhs.m_sideVectors[0]) &&
           linal::is_equal(m_sideVectors[1], rhs.m_sideVectors[1]) && linal::is_equal(m_sideVectors[2], rhs.m_sideVectors[2]);
  }
  constexpr bool operator!=(const Cuboid& rhs) const noexcept { return !(rhs == *this); }

private:
  linal::vec3<T> m_origin;
  linal::vec3Array<T, 3> m_sideVectors;
};

template <typename T>
GEO_NODISCARD constexpr linal::vec3Array<T, 8> calc_cuboid_vertices(const Cuboid<T>& cuboid) noexcept
{
  linal::vec3<T> origin = cuboid.get_origin();
  const auto& sideVecs = cuboid.get_side_vectors();
  linal::vec3Array<T, 8> vertices;
  const auto bottomX = sideVecs[0];
  const auto bottomY = sideVecs[1];
  const auto bottomZ = sideVecs[2];
  vertices[0] = linal::vec3<T>{origin};
  vertices[1] = linal::vec3<T>{origin + bottomX};
  vertices[2] = linal::vec3<T>{origin + bottomX + bottomY};
  vertices[3] = linal::vec3<T>{origin + bottomY};
  vertices[4] = linal::vec3<T>{origin + bottomZ};
  vertices[5] = linal::vec3<T>{origin + bottomX + bottomZ};
  vertices[6] = linal::vec3<T>{origin + bottomX + bottomY + bottomZ};
  vertices[7] = linal::vec3<T>{origin + bottomY + bottomZ};
  return vertices;
}

} // namespace Geometry

#endif // GEOMETRY_CUBE_H
