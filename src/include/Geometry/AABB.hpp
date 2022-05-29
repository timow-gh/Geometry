#ifndef GLFWTESTAPP_AABB_H
#define GLFWTESTAPP_AABB_H

#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t Dim>
class AABB {
  public:
    CORE_CONSTEXPR AABB(const LinAl::Vec<T, Dim>& min, const LinAl::Vec<T, Dim>& max) : m_min(min), m_max(max) {}

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, Dim>& min() const { return m_min; }
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, Dim>& max() const { return m_max; }

  protected:
    LinAl::Vec<T, Dim> m_min;
    LinAl::Vec<T, Dim> m_max;
};

} // namespace Geometry

#endif