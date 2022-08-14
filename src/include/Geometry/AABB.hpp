#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include <Core/Types/TArray.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t N>
class AABB {
    LinAl::Vec<TFloat, N> m_origin;
    Core::TArray<TFloat, N> m_extend;

  public:
    CORE_CONSTEXPR AABB(LinAl::Vec<TFloat, N> origin, TFloat extend) : m_origin(origin) { m_extend.fill(extend); }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec<TFloat, N> getOrigin() const { return m_origin; }
    CORE_CONSTEXPR void setOrigin(LinAl::Vec<TFloat, N> origin) { m_origin = origin; }

    CORE_NODISCARD CORE_CONSTEXPR const Core::TArray<TFloat, N>& getExtend() const { return m_extend; }
    CORE_CONSTEXPR void setExtend(const Core::TArray<TFloat, N>& extend) { m_extend = extend; }
};

class AABB2f : public AABB<float_t, 2> {
  public:
    using AABB<float_t, 2>::AABB;
};

class AABB3f : public AABB<float_t, 3> {
  public:
    using AABB<float_t, 3>::AABB;
};

class AABB2d : public AABB<double_t, 2> {
  public:
    using AABB<double_t, 2>::AABB;
};

class AABB3d : public AABB<double_t, 3> {
  public:
    using AABB<double_t, 3>::AABB;
};

} // namespace Geometry

#endif // GEOMETRY_AABB_HPP
