#ifndef GLFWTESTAPP_AABB_H
#define GLFWTESTAPP_AABB_H

#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t Dim>
class AABB {
  public:
    CORE_CONSTEXPR AABB(const LinAl::Vec<T, Dim>& min,
                        const LinAl::Vec<T, Dim>& max);

    inline const LinAl::Vec<T, Dim>& min() const;
    inline LinAl::Vec<T, Dim>& min();

    inline const LinAl::Vec<T, Dim>& max() const;
    inline LinAl::Vec<T, Dim>& max();

  protected:
    LinAl::Vec<T, Dim> m_min;
    LinAl::Vec<T, Dim> m_max;
};

template <typename T, std::size_t Dim>
CORE_CONSTEXPR AABB<T, Dim>::AABB(const LinAl::Vec<T, Dim>& min,
                                  const LinAl::Vec<T, Dim>& max)
    : m_min(min), m_max(max)
{
}

template <typename T, std::size_t Dim>
const LinAl::Vec<T, Dim>& AABB<T, Dim>::min() const
{
    return m_min;
}

template <typename T, std::size_t Dim>
LinAl::Vec<T, Dim>& AABB<T, Dim>::min()
{
    return m_min;
}

template <typename T, std::size_t Dim>
const LinAl::Vec<T, Dim>& AABB<T, Dim>::max() const
{
    return m_max;
}

template <typename T, std::size_t Dim>
LinAl::Vec<T, Dim>& AABB<T, Dim>::max()
{
    return m_max;
}

} // namespace Geometry

#endif