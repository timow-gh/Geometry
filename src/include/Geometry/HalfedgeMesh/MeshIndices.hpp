#ifndef MESHLER_HALFEDGEMESHINDICES_HPP
#define MESHLER_HALFEDGEMESHINDICES_HPP

#include "Geometry/Handle.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>

namespace Geometry
{

template <typename T>
class VertexIndex : public Handle<T> {
public:
  using Handle<T>::Handle;
};

template <typename T>
class HalfedgeIndex : public Handle<T> {
public:
  using Handle<T>::Handle;
};

template <typename T>
class FacetIndex : public Handle<T> {
public:
  using Handle<T>::Handle;
};

} // namespace Geometry

#endif // MESHLER_HALFEDGEMESHINDICES_HPP
