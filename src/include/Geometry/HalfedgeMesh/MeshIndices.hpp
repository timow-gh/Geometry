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
  using Handle<T>::invalidHandles;

  VertexIndex() noexcept
      : Handle<T>(Handle<T>::invalidHandles)
  {
  }

  VertexIndex(T value) noexcept
      : Handle<T>(value)
  {
  }
};

template <typename T>
class HalfedgeIndex : public Handle<T> {
public:
  using Handle<T>::invalidHandles;

  HalfedgeIndex() noexcept
      : Handle<T>(Handle<T>::invalidHandles)
  {
  }

  HalfedgeIndex(T value) noexcept
      : Handle<T>(value)
  {
  }
};

template <typename T>
class FacetIndex : public Handle<T> {
public:
  using Handle<T>::invalidHandles;

  FacetIndex() noexcept
      : Handle<T>(Handle<T>::invalidHandles)
  {
  }

  FacetIndex(T value) noexcept
      : Handle<T>(value)
  {
  }
};

} // namespace Geometry

#endif // MESHLER_HALFEDGEMESHINDICES_HPP
