#ifndef GLFWTESTAPP_PLANE_HPP
#define GLFWTESTAPP_PLANE_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T>
class Plane {
  private:
    LinAl::Vec3<T> m_origin;
    LinAl::Vec3<T> m_normal;

  public:
    CORE_CONSTEXPR Plane() = default;
    CORE_CONSTEXPR Plane(const LinAl::Vec3<T>& origin, const LinAl::Vec3<T>& normal)
        : m_origin(origin), m_normal(LinAl::normalize(normal))
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const { return m_origin; }
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getNormal() const { return m_normal; }

    CORE_CONSTEXPR bool operator==(const Plane& rhs) const { return m_origin == rhs.m_origin && m_normal == rhs.m_normal; }
    CORE_CONSTEXPR bool operator!=(const Plane& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_PLANE_HPP
