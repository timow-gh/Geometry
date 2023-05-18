#ifndef FILAPP_CIRCLE_HPP
#define FILAPP_CIRCLE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <linal/HMat.hpp>
#include <linal/HMatRotation.hpp>
#include <linal/HMatTranslation.hpp>
#include <linal/HVec.hpp>
#include <linal/Vec2.hpp>
#include <linal/Vec3.hpp>
#include <linal/VecOperations.hpp>
#include <linal/utils/Eps.hpp>

namespace Geometry
{

template <typename T>
class Circle2 {
  linal::Vec2<T> m_origin;
  T m_radius;

public:
  GEO_CONSTEXPR Circle2(const linal::Vec2<T>& origin, T radius)
      : m_origin(origin)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec2<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR T getRadius() const { return m_radius; }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const Circle2& rhs) const { return m_origin == rhs.m_origin && m_radius == rhs.m_radius; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const Circle2& rhs) const { return !(rhs == *this); }
};

using Circle2f = Circle2<float>;
using Circle2d = Circle2<double>;

template <typename T>
class Circle3 {
  linal::Vec3<T> m_origin;
  T m_radius;
  linal::Vec3<T> m_normal;

public:
  GEO_CONSTEXPR Circle3(const linal::Vec3<T>& origin, T radius, const linal::Vec3<T>& normal)
      : m_origin(origin)
      , m_radius(radius)
      , m_normal(normal)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR T getRadius() const { return m_radius; }
  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec3<T>& getNormal() const { return m_normal; }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const Circle3& rhs) const
  {
    return m_origin == rhs.m_origin && m_radius == rhs.m_radius && m_normal == rhs.m_normal;
  }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const Circle3& rhs) const { return !(rhs == *this); }
};

template <typename T>
class Circle3;
using Circle3d = Circle3<double>;

using Circle3f = Circle3<float>;
using Circle3d = Circle3<double>;

} // namespace Geometry

#endif // FILAPP_CIRCLE_HPP
