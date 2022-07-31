#ifndef GLFWTESTAPP_LINE_H
#define GLFWTESTAPP_LINE_H

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Fwd/FwdLine.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class Line {
    LinAl::Vec<T, D> m_origin;
    LinAl::Vec<T, D> m_direction;

  public:
    CORE_CONSTEXPR Line(const LinAl::Vec<T, D>& origin, const LinAl::Vec<T, D>& direction)
        : m_origin(origin), m_direction(LinAl::normalize(LinAl::Vec<T, D>{direction}))
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getOrigin() const { return m_origin; }
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getDirection() const { return m_direction; }
};

} // namespace Geometry

#endif // GLFWTESTAPP_LINE_H
