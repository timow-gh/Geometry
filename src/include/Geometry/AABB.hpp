#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include <Core/Types/TArray.hpp>
#include <Geometry/Fwd/FwdAABB.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
class AABB {
    LinAl::Vec<TFloat, D> m_origin;
    Core::TArray<TFloat, D> m_extend;

  public:
    CORE_CONSTEXPR AABB(LinAl::Vec<TFloat, D> origin, TFloat extend) : m_origin(origin) { m_extend.fill(extend); }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec<TFloat, D> getOrigin() const { return m_origin; }
    CORE_CONSTEXPR void setOrigin(LinAl::Vec<TFloat, D> origin) { m_origin = origin; }

    CORE_NODISCARD CORE_CONSTEXPR const Core::TArray<TFloat, D>& getExtend() const { return m_extend; }
    CORE_CONSTEXPR void setExtend(const Core::TArray<TFloat, D>& extend) { m_extend = extend; }
};

} // namespace Geometry

#endif // GEOMETRY_AABB_HPP
