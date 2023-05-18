#ifndef FILAPP_CIRCLE_HPP
#define FILAPP_CIRCLE_HPP

#include <Core/Math/Eps.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Circle2 {
  LinAl::Vec2<T> m_origin;
  T m_radius;

public:
  GEO_CONSTEXPR Circle2(const LinAl::Vec2<T>& origin, T radius)
      : m_origin(origin)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec2<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR T getRadius() const { return m_radius; }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const Circle2& rhs) const { return m_origin == rhs.m_origin && m_radius == rhs.m_radius; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const Circle2& rhs) const { return !(rhs == *this); }
};

using Circle2f = Circle2<float>;
using Circle2d = Circle2<double>;

template <typename T>
class Circle3 {
  LinAl::Vec3<T> m_origin;
  T m_radius;
  LinAl::Vec3<T> m_normal;

public:
  GEO_CONSTEXPR Circle3(const LinAl::Vec3<T>& origin, T radius, const LinAl::Vec3<T>& normal)
      : m_origin(origin)
      , m_radius(radius)
      , m_normal(normal)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR T getRadius() const { return m_radius; }
  GEO_NODISCARD GEO_CONSTEXPR const LinAl::Vec3<T>& getNormal() const { return m_normal; }

  GEO_NODISCARD GEO_CONSTEXPR LinAl::HMatrix<T> calcTransformation() const
  {
    LinAl::HMatrix<T> transformation =
        LinAl::hMatRotationAlign(LinAl::HVec<T>{0, 0, 1, 1}, LinAl::HVec<T>{m_normal[0], m_normal[1], m_normal[2], 1});
    LinAl::setTranslation(transformation, m_origin);
    return transformation;
  }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const Circle3& rhs) const
  {
    return m_origin == rhs.m_origin && m_radius == rhs.m_radius && m_normal == rhs.m_normal;
  }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const Circle3& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // FILAPP_CIRCLE_HPP
