#ifndef GEOMETRY_MESH_MESHRESULT_HPP
#define GEOMETRY_MESH_MESHRESULT_HPP

#include "Geometry/Utils/Compiler.hpp"

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Shared success predicate for the mesh result wrappers (\c MeshBufferResult,
 * \c HalfedgeNormals).
 *
 * Each wrapper stays a plain aggregate -- so \c {payload,error} brace-init keeps working -- and
 * forwards its \c has_value() / \c operator \c bool() here, keeping the "is this the Ok value" rule
 * in one place instead of duplicating it per wrapper.
 */
template <typename TStatus>
GEO_NODISCARD constexpr bool mesh_result_ok(TStatus status) noexcept
{
  return status == TStatus::Ok;
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_MESHRESULT_HPP
