#ifndef GEOMETRY_MESH_TRIANGLEHALFEDGEMESH_HPP
#define GEOMETRY_MESH_TRIANGLEHALFEDGEMESH_HPP

#include "Geometry/Handle.hpp"
#include "Geometry/Mesh/MeshConnectivity.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/Utils/Constness.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <linal/vec.hpp>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <compare>

namespace Geometry
{

template <typename C>
concept MeshCirculator = requires(C c) {
  { *c };
  { ++c } -> std::same_as<C&>;
  { c.is_valid() } -> std::same_as<bool>;
  { static_cast<bool>(c) };
};

template <typename T, std::uint8_t D, typename TIndex = std::uint32_t>
class TriangleHalfedgeMesh {
public:
  struct VertexHandleTag;
  struct HalfedgeHandleTag;
  struct FaceHandleTag;
  struct EdgeHandleTag;

  using value_type = T;
  using handle_value_type = TIndex;
  using size_type = std::size_t;
  using vec_t = linal::vec<T, D>;

  using VertexHandle = Handle<handle_value_type, VertexHandleTag>;
  using HalfedgeHandle = Handle<handle_value_type, HalfedgeHandleTag>;
  using FaceHandle = Handle<handle_value_type, FaceHandleTag>;
  using EdgeHandle = Handle<handle_value_type, EdgeHandleTag>;

  struct Vertex
  {
    vec_t position{};
    HalfedgeHandle halfedge{};
  };

  struct Halfedge
  {
    // The face is on the left when walking source -> target.
    VertexHandle targetVertex{};
    HalfedgeHandle twin{};
    HalfedgeHandle next{};
    HalfedgeHandle prev{};
    FaceHandle face{};
    EdgeHandle edge{};

    // A halfedge is a boundary halfedge iff it carries no incident face.
    GEO_NODISCARD constexpr bool is_boundary() const noexcept { return !face.is_valid(); }
  };

  class Face
  {
    HalfedgeHandle m_heHandle{};

  public:
    constexpr Face() noexcept = default;
    constexpr explicit Face(HalfedgeHandle heHandle) noexcept
        : m_heHandle(heHandle) {}

    [[nodiscard]] constexpr bool operator==(const Face& other) const noexcept { return m_heHandle == other.m_heHandle; }
    [[nodiscard]] constexpr bool operator!=(const Face& other) const noexcept { return !(*this == other); }

    constexpr std::strong_ordering operator<=>(const Face& other) const noexcept {
        return m_heHandle.get_value() <=> other.m_heHandle.get_value();
    }

    [[nodiscard]] constexpr const HalfedgeHandle get_halfedgehandle() const noexcept { return m_heHandle; }
    constexpr void set_halfedgehandle(HalfedgeHandle heHandle) noexcept { m_heHandle = heHandle; }
  };

  struct Edge
  {
    HalfedgeHandle halfedge{};
  };

  template <typename Mesh, Constness C>
  friend class MeshConnectivityView;

  using ConnectivityView = MeshConnectivityView<TriangleHalfedgeMesh, Constness::Mutable>;
  using ConstConnectivityView = MeshConnectivityView<TriangleHalfedgeMesh, Constness::Const>;

  // Explicit, opt-in accessor for the low-level connectivity kernel. Write algorithms
  // obtain the unchecked mutation primitives through this handle
  GEO_NODISCARD ConnectivityView connectivity() noexcept { return ConnectivityView{this}; }
  GEO_NODISCARD ConstConnectivityView connectivity() const noexcept { return ConstConnectivityView{this}; }

  template <Constness C>
  using MeshPtrT = qualified_ptr_t<C, TriangleHalfedgeMesh>;

  // Shared scaffolding for all mesh circulators (CRTP). Holds the traversal state and every member
  // that is identical across circulators: start-tracking, is_valid/operator bool, equality, and both
  // operator++ forms. Each Derived supplies only what actually differs:
  //   * static HalfedgeHandle advance_step(mesh_pointer, HalfedgeHandle) -- one step of the orbit
  //     (`next` for face circulators, `twin.next` for vertex circulators);
  //   * optionally a skip() member (invoked after construction and after each advance) for circulators
  //     that must skip boundary halfedges (FaceFace, VertexFace); the default is a no-op;
  //   * the dereference / get_* accessors, which depend on what the circulator yields.
  // Derived is the fully-qualified circulator type for the SAME Constness, so CRTP can downcast for the
  // skip() hook and so operator++ returns the Derived type.
  template <typename Derived, Constness C>
  class CirculatorBaseT
  {
    protected:
      MeshPtrT<C> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

      // No-op skip by default; circulators that skip boundaries hide this with their own skip().
      constexpr void skip() noexcept {}

      [[nodiscard]] constexpr Derived& derived() noexcept { return static_cast<Derived&>(*this); }
      [[nodiscard]] constexpr const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

    public:
      using mesh_pointer = MeshPtrT<C>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr CirculatorBaseT() noexcept = default;
      constexpr CirculatorBaseT(HalfedgeHandle heHandle, MeshPtrT<C> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      // Non-const -> const conversion only, from the base of the non-const Derived. The
      // is_const(C) && !is_const(Other) constraint keeps this from shadowing the implicit
      // copy/move ctors (which would break std::semiregular).
      template <typename OtherDerived, Constness Other>
      constexpr CirculatorBaseT(const CirculatorBaseT<OtherDerived, Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr Derived& operator++() {
        const HalfedgeHandle nextHe = Derived::advance_step(m_mesh, m_heHandle);
        assert(nextHe.is_valid());
        m_heHandle = nextHe;
        m_started = true;
        derived().skip();
        return derived();
      }

      constexpr Derived operator++(int) {
        Derived old = derived();
        ++(*this);
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const CirculatorBaseT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const CirculatorBaseT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }

      template <typename, Constness>
      friend class CirculatorBaseT;
  };

  template <Constness C>
  class FaceHalfedgeCirculatorT : public CirculatorBaseT<FaceHalfedgeCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<FaceHalfedgeCirculatorT<C>, C>;
      friend Base;

      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(heHandle).next;
      }

    public:
      using value_type = Halfedge;
      using reference = qualified_ref_t<C, Halfedge>;
      using pointer = qualified_ptr_t<C, Halfedge>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      using Base::Base;
      constexpr FaceHalfedgeCirculatorT() noexcept = default;
      template <Constness Other>
      constexpr FaceHalfedgeCirculatorT(const FaceHalfedgeCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_halfedge(this->m_heHandle); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_halfedge(this->m_heHandle); }
  };

  template <Constness C>
  class FaceVertexCirculatorT : public CirculatorBaseT<FaceVertexCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<FaceVertexCirculatorT<C>, C>;
      friend Base;

      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(heHandle).next;
      }

    public:
      using value_type = Vertex;
      using reference = qualified_ref_t<C, Vertex>;
      using pointer = qualified_ptr_t<C, Vertex>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      using Base::Base;
      constexpr FaceVertexCirculatorT() noexcept = default;
      template <Constness Other>
      constexpr FaceVertexCirculatorT(const FaceVertexCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_vertex(this->m_mesh->target_vertex(this->m_heHandle)); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_vertex(this->m_mesh->target_vertex(this->m_heHandle)); }

      [[nodiscard]] constexpr VertexHandle get_vertexhandle() const noexcept { return this->m_mesh->target_vertex(this->m_heHandle); }
  };

  template <Constness C>
  class FaceFaceCirculatorT : public CirculatorBaseT<FaceFaceCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<FaceFaceCirculatorT<C>, C>;
      friend Base;

      // The step around the face is `next`; the neighbour lives across the twin. Skipping keeps the
      // cursor off boundary edges (whose twin has no face).
      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(heHandle).next;
      }

      [[nodiscard]] constexpr bool cursor_is_boundary() const noexcept
      {
        return this->m_mesh->get_halfedge(this->m_mesh->get_halfedge(this->m_heHandle).twin).is_boundary();
      }

      [[nodiscard]] constexpr FaceHandle neighbor_face() const noexcept {
        return this->m_mesh->get_halfedge(this->m_mesh->get_halfedge(this->m_heHandle).twin).face;
      }

    protected:
      // Advance along 'next' across boundary edges so the circulator emits only real neighbour faces.
      constexpr void skip() noexcept {
        while (this->m_heHandle.is_valid() && !this->has_returned_to_start() && cursor_is_boundary())
        {
          this->m_heHandle = this->m_mesh->get_halfedge(this->m_heHandle).next;
          this->m_started = true;
        }
      }

    public:
      using value_type = Face;
      using reference = qualified_ref_t<C, Face>;
      using pointer = qualified_ptr_t<C, Face>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      constexpr FaceFaceCirculatorT() noexcept = default;
      constexpr FaceFaceCirculatorT(HalfedgeHandle heHandle, MeshPtrT<C> mesh) noexcept
          : Base(heHandle, mesh) {
        if (this->m_mesh != nullptr)
        {
          skip();
        }
      }

      template <Constness Other>
      constexpr FaceFaceCirculatorT(const FaceFaceCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_face(neighbor_face()); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_face(neighbor_face()); }

      [[nodiscard]] constexpr FaceHandle get_facehandle() const noexcept { return neighbor_face(); }
  };

  template <Constness C>
  class VertexOutHalfedgeCirculatorT : public CirculatorBaseT<VertexOutHalfedgeCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<VertexOutHalfedgeCirculatorT<C>, C>;
      friend Base;

      // Every halfedge has a valid twin, so rotating around the vertex via twin.next always closes.
      // Boundary halfedges are part of the fan (they carry the boundary loop), so circulation is
      // uniform and terminates only by returning to the start.
      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(mesh->get_halfedge(heHandle).twin).next;
      }

    public:
      using value_type = Halfedge;
      using reference = qualified_ref_t<C, Halfedge>;
      using pointer = qualified_ptr_t<C, Halfedge>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      using Base::Base;
      constexpr VertexOutHalfedgeCirculatorT() noexcept = default;
      template <Constness Other>
      constexpr VertexOutHalfedgeCirculatorT(const VertexOutHalfedgeCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_halfedge(this->m_heHandle); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_halfedge(this->m_heHandle); }
  };

  template <Constness C>
  class VertexInHalfedgeCirculatorT : public CirculatorBaseT<VertexInHalfedgeCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<VertexInHalfedgeCirculatorT<C>, C>;
      friend Base;

      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(mesh->get_halfedge(heHandle).twin).next;
      }

      [[nodiscard]] constexpr HalfedgeHandle incoming_halfedge() const noexcept {
        return this->m_mesh->get_halfedge(this->m_heHandle).prev;
      }

    public:
      using value_type = Halfedge;
      using reference = qualified_ref_t<C, Halfedge>;
      using pointer = qualified_ptr_t<C, Halfedge>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      using Base::Base;
      constexpr VertexInHalfedgeCirculatorT() noexcept = default;
      template <Constness Other>
      constexpr VertexInHalfedgeCirculatorT(const VertexInHalfedgeCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_halfedge(incoming_halfedge()); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_halfedge(incoming_halfedge()); }

      [[nodiscard]] constexpr HalfedgeHandle get_halfedgehandle() const noexcept { return incoming_halfedge(); }
      [[nodiscard]] constexpr const HalfedgeHandle& get_outgoinghalfedgehandle() const noexcept { return this->m_heHandle; }
  };

  template <Constness C>
  class VertexVertexCirculatorT : public CirculatorBaseT<VertexVertexCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<VertexVertexCirculatorT<C>, C>;
      friend Base;

      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(mesh->get_halfedge(heHandle).twin).next;
      }

    public:
      using value_type = Vertex;
      using reference = qualified_ref_t<C, Vertex>;
      using pointer = qualified_ptr_t<C, Vertex>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      using Base::Base;
      constexpr VertexVertexCirculatorT() noexcept = default;
      template <Constness Other>
      constexpr VertexVertexCirculatorT(const VertexVertexCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_vertex(this->m_mesh->target_vertex(this->m_heHandle)); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_vertex(this->m_mesh->target_vertex(this->m_heHandle)); }

      [[nodiscard]] constexpr VertexHandle get_vertexhandle() const noexcept { return this->m_mesh->target_vertex(this->m_heHandle); }
  };

  template <Constness C>
  class VertexFaceCirculatorT : public CirculatorBaseT<VertexFaceCirculatorT<C>, C>
  {
      using Base = CirculatorBaseT<VertexFaceCirculatorT<C>, C>;
      friend Base;

      static constexpr HalfedgeHandle advance_step(MeshPtrT<C> mesh, HalfedgeHandle heHandle) noexcept {
        return mesh->get_halfedge(mesh->get_halfedge(heHandle).twin).next;
      }

      [[nodiscard]] constexpr FaceHandle incident_face() const noexcept { return this->m_mesh->get_halfedge(this->m_heHandle).face; }

      [[nodiscard]] constexpr bool cursor_is_boundary() const noexcept { return this->m_mesh->get_halfedge(this->m_heHandle).is_boundary(); }

    protected:
      // A vertex-face circulator visits incident faces only. Boundary (no-face) halfedges are part of
      // the fan but have no face, so skip them by rotating further around the vertex.
      constexpr void skip() noexcept {
        while (this->m_heHandle.is_valid() && !this->has_returned_to_start() && cursor_is_boundary())
        {
          this->m_heHandle = advance_step(this->m_mesh, this->m_heHandle);
          this->m_started = true;
        }
      }

    public:
      using value_type = Face;
      using reference = qualified_ref_t<C, Face>;
      using pointer = qualified_ptr_t<C, Face>;
      using mesh_pointer = typename Base::mesh_pointer;
      using size_type = typename Base::size_type;
      using difference_type = typename Base::difference_type;

      constexpr VertexFaceCirculatorT() noexcept = default;
      constexpr VertexFaceCirculatorT(HalfedgeHandle heHandle, MeshPtrT<C> mesh) noexcept
          : Base(heHandle, mesh) {
        if (this->m_mesh != nullptr)
        {
          skip();
        }
      }

      template <Constness Other>
      constexpr VertexFaceCirculatorT(const VertexFaceCirculatorT<Other>& other) noexcept
          requires(is_const(C) && !is_const(Other))
          : Base(other) {}

      constexpr pointer operator->() const noexcept { return &this->m_mesh->get_face(incident_face()); }
      constexpr reference operator*() const noexcept { return this->m_mesh->get_face(incident_face()); }

      [[nodiscard]] constexpr FaceHandle get_facehandle() const noexcept { return incident_face(); }
  };

  using ConstFaceHalfedgeCirculator = FaceHalfedgeCirculatorT<Constness::Const>;
  using FaceHalfedgeCirculator = FaceHalfedgeCirculatorT<Constness::Mutable>;
  using ConstFaceVertexCirculator = FaceVertexCirculatorT<Constness::Const>;
  using FaceVertexCirculator = FaceVertexCirculatorT<Constness::Mutable>;
  using ConstFaceFaceCirculator = FaceFaceCirculatorT<Constness::Const>;
  using FaceFaceCirculator = FaceFaceCirculatorT<Constness::Mutable>;

  using ConstVertexOutHalfedgeCirculator = VertexOutHalfedgeCirculatorT<Constness::Const>;
  using VertexOutHalfedgeCirculator = VertexOutHalfedgeCirculatorT<Constness::Mutable>;
  using ConstVertexInHalfedgeCirculator = VertexInHalfedgeCirculatorT<Constness::Const>;
  using VertexInHalfedgeCirculator = VertexInHalfedgeCirculatorT<Constness::Mutable>;
  using ConstVertexVertexCirculator = VertexVertexCirculatorT<Constness::Const>;
  using VertexVertexCirculator = VertexVertexCirculatorT<Constness::Mutable>;
  using ConstVertexFaceCirculator = VertexFaceCirculatorT<Constness::Const>;
  using VertexFaceCirculator = VertexFaceCirculatorT<Constness::Mutable>;

  template <typename Circulator>
  struct FaceCirculatorSentinel
  {
      [[nodiscard]] friend constexpr bool operator==(const Circulator& circ, FaceCirculatorSentinel) noexcept {
        return !circ.is_valid();
      }
  };

  using FaceHalfedgeSentinel = FaceCirculatorSentinel<ConstFaceHalfedgeCirculator>;

  template <typename Circulator>
  class FaceCirculatorRange
  {
      using MeshPtr = typename Circulator::mesh_pointer;

      MeshPtr m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};

    public:
      constexpr FaceCirculatorRange() noexcept = default;
      constexpr explicit FaceCirculatorRange(HalfedgeHandle heHandle, MeshPtr mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle) {}

      [[nodiscard]] constexpr Circulator begin() const noexcept { return Circulator(m_heHandle, m_mesh); }
      [[nodiscard]] constexpr FaceCirculatorSentinel<Circulator> end() const noexcept { return {}; }

      [[nodiscard]] constexpr Circulator circulator() const noexcept { return Circulator(m_heHandle, m_mesh); }
  };

  using ConstFaceHalfedgeRange = FaceCirculatorRange<ConstFaceHalfedgeCirculator>;
  using FaceHalfedgeRange = FaceCirculatorRange<FaceHalfedgeCirculator>;
  using ConstFaceVertexRange = FaceCirculatorRange<ConstFaceVertexCirculator>;
  using FaceVertexRange = FaceCirculatorRange<FaceVertexCirculator>;
  using ConstFaceFaceRange = FaceCirculatorRange<ConstFaceFaceCirculator>;
  using FaceFaceRange = FaceCirculatorRange<FaceFaceCirculator>;

  using ConstVertexOutHalfedgeRange = FaceCirculatorRange<ConstVertexOutHalfedgeCirculator>;
  using VertexOutHalfedgeRange = FaceCirculatorRange<VertexOutHalfedgeCirculator>;
  using ConstVertexInHalfedgeRange = FaceCirculatorRange<ConstVertexInHalfedgeCirculator>;
  using VertexInHalfedgeRange = FaceCirculatorRange<VertexInHalfedgeCirculator>;
  using ConstVertexVertexRange = FaceCirculatorRange<ConstVertexVertexCirculator>;
  using VertexVertexRange = FaceCirculatorRange<VertexVertexCirculator>;
  using ConstVertexFaceRange = FaceCirculatorRange<ConstVertexFaceCirculator>;
  using VertexFaceRange = FaceCirculatorRange<VertexFaceCirculator>;

  template <typename THandle>
  class ElementIteratorT
  {
      handle_value_type m_index{};

    public:
      using value_type = THandle;
      using reference = THandle;
      using pointer = void;
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::random_access_iterator_tag;

      constexpr ElementIteratorT() noexcept = default;
      constexpr explicit ElementIteratorT(handle_value_type index) noexcept
          : m_index(index) {}

      constexpr THandle operator*() const noexcept { return THandle{m_index}; }

      constexpr ElementIteratorT& operator++() noexcept {
        ++m_index;
        return *this;
      }

      constexpr ElementIteratorT operator++(int) noexcept {
        ElementIteratorT old = *this;
        ++m_index;
        return old;
      }

      constexpr ElementIteratorT& operator--() noexcept {
        --m_index;
        return *this;
      }

      constexpr ElementIteratorT operator--(int) noexcept {
        ElementIteratorT old = *this;
        --m_index;
        return old;
      }

      constexpr ElementIteratorT& operator+=(difference_type offset) noexcept {
        m_index = static_cast<handle_value_type>(static_cast<difference_type>(m_index) + offset);
        return *this;
      }

      constexpr ElementIteratorT& operator-=(difference_type offset) noexcept { return *this += -offset; }

      [[nodiscard]] friend constexpr ElementIteratorT operator+(ElementIteratorT iterator, difference_type offset) noexcept {
        iterator += offset;
        return iterator;
      }

      [[nodiscard]] friend constexpr ElementIteratorT operator+(difference_type offset, ElementIteratorT iterator) noexcept {
        iterator += offset;
        return iterator;
      }

      [[nodiscard]] friend constexpr ElementIteratorT operator-(ElementIteratorT iterator, difference_type offset) noexcept {
        iterator -= offset;
        return iterator;
      }

      [[nodiscard]] friend constexpr difference_type operator-(ElementIteratorT lhs, ElementIteratorT rhs) noexcept {
        return static_cast<difference_type>(lhs.m_index) - static_cast<difference_type>(rhs.m_index);
      }

      [[nodiscard]] constexpr THandle operator[](difference_type offset) const noexcept { return *(*this + offset); }

      [[nodiscard]] constexpr bool operator==(const ElementIteratorT& other) const noexcept { return m_index == other.m_index; }
      [[nodiscard]] constexpr bool operator!=(const ElementIteratorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr std::strong_ordering operator<=>(const ElementIteratorT& other) const noexcept {
        return m_index <=> other.m_index;
      }
  };

  using VertexIterator = ElementIteratorT<VertexHandle>;
  using HalfedgeIterator = ElementIteratorT<HalfedgeHandle>;
  using EdgeIterator = ElementIteratorT<EdgeHandle>;
  using FaceIterator = ElementIteratorT<FaceHandle>;

  template <typename THandle>
  class ElementRangeT
  {
      handle_value_type m_begin{};
      handle_value_type m_end{};

    public:
      constexpr ElementRangeT() noexcept = default;
      constexpr ElementRangeT(handle_value_type first, handle_value_type last) noexcept
          : m_begin(first)
          , m_end(last) {}

      [[nodiscard]] constexpr ElementIteratorT<THandle> begin() const noexcept { return ElementIteratorT<THandle>{m_begin}; }
      [[nodiscard]] constexpr ElementIteratorT<THandle> end() const noexcept { return ElementIteratorT<THandle>{m_end}; }

      [[nodiscard]] constexpr size_type size() const noexcept { return static_cast<size_type>(m_end - m_begin); }
      [[nodiscard]] constexpr bool empty() const noexcept { return m_begin == m_end; }
  };

  using VertexRange = ElementRangeT<VertexHandle>;
  using HalfedgeRange = ElementRangeT<HalfedgeHandle>;
  using EdgeRange = ElementRangeT<EdgeHandle>;
  using FaceRange = ElementRangeT<FaceHandle>;

  GEO_NODISCARD VertexHandle add_vertex(const vec_t& position)
  {
    const VertexHandle handle = make_handle<VertexHandle>(m_vertices.size());
    m_vertices.reserve(m_vertices.size() + 1);
    m_vertices.push_back(Vertex{position, HalfedgeHandle{}});
    return handle;
  }

  GEO_NODISCARD constexpr size_type vertex_count() const noexcept { return m_vertices.size(); }
  GEO_NODISCARD constexpr size_type halfedge_count() const noexcept { return m_halfedges.size(); }
  GEO_NODISCARD constexpr size_type face_count() const noexcept { return m_faces.size(); }
  GEO_NODISCARD constexpr size_type edge_count() const noexcept { return m_edges.size(); }
  GEO_NODISCARD constexpr bool empty() const noexcept { return m_vertices.empty() && m_faces.empty(); }

  GEO_NODISCARD VertexRange vertices() const noexcept
  {
    return VertexRange{0, static_cast<handle_value_type>(vertex_count())};
  }

  GEO_NODISCARD HalfedgeRange halfedges() const noexcept
  {
    return HalfedgeRange{0, static_cast<handle_value_type>(halfedge_count())};
  }

  GEO_NODISCARD EdgeRange edges() const noexcept
  {
    return EdgeRange{0, static_cast<handle_value_type>(edge_count())};
  }

  GEO_NODISCARD FaceRange faces() const noexcept
  {
    return FaceRange{0, static_cast<handle_value_type>(face_count())};
  }

  GEO_NODISCARD bool contains(VertexHandle handle) const noexcept { return handle_in_range(handle, m_vertices.size()); }
  GEO_NODISCARD bool contains(HalfedgeHandle handle) const noexcept { return handle_in_range(handle, m_halfedges.size()); }
  GEO_NODISCARD bool contains(FaceHandle handle) const noexcept { return handle_in_range(handle, m_faces.size()); }
  GEO_NODISCARD bool contains(EdgeHandle handle) const noexcept { return handle_in_range(handle, m_edges.size()); }

  GEO_NODISCARD Vertex& get_vertex(VertexHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_vertices[handle_index(handle)];
  }

  GEO_NODISCARD const Vertex& get_vertex(VertexHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_vertices[handle_index(handle)];
  }

  GEO_NODISCARD Halfedge& get_halfedge(HalfedgeHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_halfedges[handle_index(handle)];
  }

  GEO_NODISCARD const Halfedge& get_halfedge(HalfedgeHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_halfedges[handle_index(handle)];
  }

  GEO_NODISCARD Face& get_face(FaceHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_faces[handle_index(handle)];
  }

  GEO_NODISCARD const Face& get_face(FaceHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_faces[handle_index(handle)];
  }

  GEO_NODISCARD Edge& get_edge(EdgeHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_edges[handle_index(handle)];
  }

  GEO_NODISCARD const Edge& get_edge(EdgeHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_edges[handle_index(handle)];
  }

  GEO_NODISCARD const vec_t& get_position(VertexHandle handle) const noexcept { return get_vertex(handle).position; }

  void set_position(VertexHandle handle, const vec_t& position) noexcept { get_vertex(handle).position = position; }

  GEO_NODISCARD ConstFaceHalfedgeRange halfedges(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));
    return ConstFaceHalfedgeRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceHalfedgeRange halfedges(FaceHandle face) noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceHalfedgeRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD ConstFaceVertexRange vertices(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));
    return ConstFaceVertexRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceVertexRange vertices(FaceHandle face) noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceVertexRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD ConstFaceFaceRange adjacent_faces(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));
    return ConstFaceFaceRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceFaceRange adjacent_faces(FaceHandle face) noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceFaceRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD ConstVertexOutHalfedgeRange outgoing_halfedges(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return ConstVertexOutHalfedgeRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexOutHalfedgeRange outgoing_halfedges(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexOutHalfedgeRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD ConstVertexInHalfedgeRange incoming_halfedges(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return ConstVertexInHalfedgeRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexInHalfedgeRange incoming_halfedges(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexInHalfedgeRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD ConstVertexVertexRange vertices(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return ConstVertexVertexRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexVertexRange vertices(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexVertexRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD ConstVertexFaceRange faces(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return ConstVertexFaceRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexFaceRange faces(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexFaceRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD std::array<HalfedgeHandle, 3> halfedges_around_face(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));

    std::array<HalfedgeHandle, 3> halfedges{};
    halfedges[0] = get_face(face).get_halfedgehandle();
    halfedges[1] = get_halfedge(halfedges[0]).next;
    halfedges[2] = get_halfedge(halfedges[1]).next;
    GEO_ASSERT(get_halfedge(halfedges[2]).next == halfedges[0]);
    return halfedges;
  }

  GEO_NODISCARD std::array<VertexHandle, 3> vertices_around_face(FaceHandle face) const noexcept
  {
    const std::array<HalfedgeHandle, 3> halfedges = halfedges_around_face(face);
    return {source_vertex(halfedges[0]), target_vertex(halfedges[0]), target_vertex(halfedges[1])};
  }

  // Returns the interior (face-bearing) outgoing halfedges incident to the vertex, one per incident
  // face, in fan order. The mesh is manifold, so a single twin.next orbit visits the whole fan; there
  // is no allocation beyond the result and no per-mesh work. Boundary (no-face) outgoing halfedges are
  // skipped so the result maps one-to-one to incident faces.
  GEO_NODISCARD std::vector<HalfedgeHandle> halfedges_around_vertex(VertexHandle vertex) const
  {
    GEO_ASSERT(contains(vertex));

    std::vector<HalfedgeHandle> result;
    const HalfedgeHandle start = get_vertex(vertex).halfedge;
    if (!start.is_valid())
    {
      return result;
    }

    HalfedgeHandle current = start;
    do
    {
      if (!get_halfedge(current).is_boundary())
      {
        result.push_back(current);
      }
      current = get_halfedge(get_halfedge(current).twin).next;
    } while (current != start);

    return result;
  }

  // Counts the interior (face-bearing) outgoing halfedges incident to the vertex, i.e. the number of
  // incident faces, without allocating. Same twin.next orbit as halfedges_around_vertex; used where
  // only the count is needed (e.g. valence).
  GEO_NODISCARD size_type count_incident_faces(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));

    const HalfedgeHandle start = get_vertex(vertex).halfedge;
    if (!start.is_valid())
    {
      return 0;
    }

    size_type count = 0;
    HalfedgeHandle current = start;
    do
    {
      if (!get_halfedge(current).is_boundary())
      {
        ++count;
      }
      current = get_halfedge(get_halfedge(current).twin).next;
    } while (current != start);

    return count;
  }

  GEO_NODISCARD std::vector<FaceHandle> faces_around_vertex(VertexHandle vertex) const
  {
    GEO_ASSERT(contains(vertex));

    std::vector<FaceHandle> result;
    const HalfedgeHandle start = get_vertex(vertex).halfedge;
    if (!start.is_valid())
    {
      return result;
    }

    // Single fan walk: push each interior halfedge's face directly, avoiding the intermediate
    // halfedge vector that a two-step (halfedges_around_vertex then map) approach would allocate.
    HalfedgeHandle current = start;
    do
    {
      if (!get_halfedge(current).is_boundary())
      {
        result.push_back(get_halfedge(current).face);
      }
      current = get_halfedge(get_halfedge(current).twin).next;
    } while (current != start);

    return result;
  }

  GEO_NODISCARD VertexHandle source_vertex(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    const HalfedgeHandle prev = get_halfedge(halfedge).prev;
    GEO_ASSERT(contains(prev));
    return get_halfedge(prev).targetVertex;
  }

  GEO_NODISCARD VertexHandle target_vertex(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    return get_halfedge(halfedge).targetVertex;
  }

  // A halfedge is a boundary halfedge iff it carries no incident face. Every halfedge always has a
  // valid twin (edges own two halfedges); the boundary is signalled by an invalid face.
  GEO_NODISCARD bool is_boundary(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    return get_halfedge(halfedge).is_boundary();
  }

  // An edge is a boundary edge iff either of its two halfedges is a boundary halfedge.
  GEO_NODISCARD bool is_boundary(EdgeHandle edge) const noexcept
  {
    GEO_ASSERT(contains(edge));
    const HalfedgeHandle halfedge = get_edge(edge).halfedge;
    return is_boundary(halfedge) || is_boundary(get_halfedge(halfedge).twin);
  }

  // True if the vertex's stored representative outgoing halfedge is a boundary halfedge, i.e. the
  // vertex lies on the boundary. Isolated vertices (no stored halfedge) return false.
  GEO_NODISCARD bool is_boundary_outgoing(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    const HalfedgeHandle stored = get_vertex(vertex).halfedge;
    return stored.is_valid() && is_boundary(stored);
  }

  // Walks the fan around `vertex` starting from an outgoing halfedge `startOutgoing` and returns the
  // first outgoing boundary (no-face) halfedge found, or an invalid handle if the vertex is interior.
  // Requires the local twin/next links around the vertex to be consistent.
  GEO_NODISCARD HalfedgeHandle find_outgoing_boundary(VertexHandle vertex, HalfedgeHandle startOutgoing) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    if (!startOutgoing.is_valid())
    {
      return HalfedgeHandle{};
    }
    HalfedgeHandle current = startOutgoing;
    do
    {
      if (get_halfedge(current).is_boundary())
      {
        return current;
      }
      current = get_halfedge(get_halfedge(current).twin).next;
    } while (current != startOutgoing);
    return HalfedgeHandle{};
  }

  // Validates internal CONNECTIVITY consistency only: handle ranges, twin involution, next/prev
  // reciprocity, face-cycle shape, and the directed-edge / face-key bijections. It deliberately does
  // NOT check topological vertex-manifoldness -- a vertex whose incident halfedges form two separate
  // fans (umbrellas meeting only at the vertex) still passes here because it only walks the single fan
  // reachable from the stored halfedge. That topological property is a separate concern checked by
  // verify_vertex_manifold() in MeshManifold.hpp; keep the two distinct.
  GEO_NODISCARD bool has_valid_connectivity() const noexcept
  {
    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      const Halfedge& halfedge = m_halfedges[i];

      // Every halfedge always has a valid twin, target vertex, next/prev, and edge. The face is valid
      // for interior halfedges and invalid for boundary halfedges.
      if (!contains(halfedge.targetVertex) || !contains(halfedge.next) || !contains(halfedge.prev)
          || !contains(halfedge.twin) || !contains(halfedge.edge))
      {
        return false;
      }
      if (halfedge.face.is_valid() && !contains(halfedge.face))
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_vertices.size(); ++i)
    {
      const VertexHandle vertex = make_handle<VertexHandle>(i);
      const HalfedgeHandle halfedge = m_vertices[i].halfedge;
      if (halfedge.is_valid() && (!contains(halfedge) || unchecked_source_vertex(halfedge) != vertex))
      {
        return false;
      }
      // Boundary convention: a boundary vertex references a boundary outgoing halfedge.
      if (halfedge.is_valid())
      {
        const HalfedgeHandle boundary = find_outgoing_boundary(vertex, halfedge);
        if (boundary.is_valid() && !m_halfedges[handle_index(halfedge)].is_boundary())
        {
          return false;
        }
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      const FaceHandle face = make_handle<FaceHandle>(i);
      const HalfedgeHandle firstHalfedge = m_faces[i].get_halfedgehandle();
      if (!contains(firstHalfedge))
      {
        return false;
      }

      const HalfedgeHandle secondHalfedge = m_halfedges[handle_index(firstHalfedge)].next;
      const HalfedgeHandle thirdHalfedge = m_halfedges[handle_index(secondHalfedge)].next;
      if (firstHalfedge == secondHalfedge || secondHalfedge == thirdHalfedge || thirdHalfedge == firstHalfedge)
      {
        return false;
      }

      if (m_halfedges[handle_index(thirdHalfedge)].next != firstHalfedge)
      {
        return false;
      }

      if (m_halfedges[handle_index(firstHalfedge)].face != face || m_halfedges[handle_index(secondHalfedge)].face != face
          || m_halfedges[handle_index(thirdHalfedge)].face != face)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_edges.size(); ++i)
    {
      const EdgeHandle edge = make_handle<EdgeHandle>(i);
      const HalfedgeHandle halfedge = m_edges[i].halfedge;
      if (!contains(halfedge) || m_halfedges[handle_index(halfedge)].edge != edge)
      {
        return false;
      }
    }

    if (m_directedEdges.size() != m_halfedges.size() || m_faceKeys.size() != m_faces.size())
    {
      return false;
    }

    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      const HalfedgeHandle halfedgeHandle = make_handle<HalfedgeHandle>(i);
      const Halfedge& halfedge = m_halfedges[i];

      if (m_halfedges[handle_index(halfedge.next)].prev != halfedgeHandle
          || m_halfedges[handle_index(halfedge.prev)].next != halfedgeHandle)
      {
        return false;
      }

      // Every halfedge has a valid twin; twin is an involution sharing the same edge and reversing the
      // endpoints.
      const Halfedge& twin = m_halfedges[handle_index(halfedge.twin)];
      if (twin.twin != halfedgeHandle || twin.edge != halfedge.edge
          || unchecked_source_vertex(halfedgeHandle) != unchecked_target_vertex(halfedge.twin)
          || unchecked_target_vertex(halfedgeHandle) != unchecked_source_vertex(halfedge.twin))
      {
        return false;
      }

      // A boundary halfedge's next/prev stay on the boundary (boundary loops carry only no-face
      // halfedges).
      if (halfedge.is_boundary())
      {
        if (!m_halfedges[handle_index(halfedge.next)].is_boundary()
            || !m_halfedges[handle_index(halfedge.prev)].is_boundary())
        {
          return false;
        }
      }

      const DirectedEdgeKey key{unchecked_source_vertex(halfedgeHandle).get_value(), unchecked_target_vertex(halfedgeHandle).get_value()};
      const auto directedEdgeIt = m_directedEdges.find(key);
      if (directedEdgeIt == m_directedEdges.end() || directedEdgeIt->second != halfedgeHandle)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      const FaceHandle face = make_handle<FaceHandle>(i);
      if (m_faceKeys.find(make_face_key(vertices_around_face(face))) == m_faceKeys.end())
      {
        return false;
      }
    }

    return true;
  }

private:
  struct DirectedEdgeKey
  {
    handle_value_type from{};
    handle_value_type to{};

    GEO_NODISCARD constexpr bool operator==(const DirectedEdgeKey& other) const noexcept
    {
      return from == other.from && to == other.to;
    }
  };

  struct DirectedEdgeKeyHash
  {
    GEO_NODISCARD size_type operator()(const DirectedEdgeKey& key) const noexcept
    {
      const size_type from = static_cast<size_type>(key.from);
      const size_type to = static_cast<size_type>(key.to);
      return from ^ (to + 0x9e3779b9U + (from << 6U) + (from >> 2U));
    }
  };

  struct FaceKey
  {
    std::array<handle_value_type, 3> vertices{};

    GEO_NODISCARD constexpr bool operator==(const FaceKey& other) const noexcept { return vertices == other.vertices; }
  };

  struct FaceKeyHash
  {
    GEO_NODISCARD size_type operator()(const FaceKey& key) const noexcept
    {
      size_type seed = 0;
      for (handle_value_type vertex : key.vertices)
      {
        const size_type value = static_cast<size_type>(vertex);
        seed ^= value + 0x9e3779b9U + (seed << 6U) + (seed >> 2U);
      }
      return seed;
    }
  };

  GEO_NODISCARD static FaceKey make_face_key(const std::array<VertexHandle, 3>& vertices) noexcept
  {
    FaceKey key{{vertices[0].get_value(), vertices[1].get_value(), vertices[2].get_value()}};
    std::sort(key.vertices.begin(), key.vertices.end());
    return key;
  }

  GEO_NODISCARD VertexHandle unchecked_source_vertex(HalfedgeHandle halfedge) const noexcept
  {
    return m_halfedges[handle_index(m_halfedges[handle_index(halfedge)].prev)].targetVertex;
  }

  GEO_NODISCARD VertexHandle unchecked_target_vertex(HalfedgeHandle halfedge) const noexcept
  {
    return m_halfedges[handle_index(halfedge)].targetVertex;
  }

  template <typename THandle>
  GEO_NODISCARD static THandle make_handle(size_type index) noexcept {
      GEO_ASSERT(index < static_cast<size_type>(std::numeric_limits<handle_value_type>::max()));
      return THandle{static_cast<handle_value_type>(index)};
  }

  template <typename THandle>
  GEO_NODISCARD static size_type handle_index(THandle handle) noexcept
  {
    return static_cast<size_type>(handle.get_value());
  }

  template <typename THandle>
  GEO_NODISCARD static bool handle_in_range(THandle handle, size_type count) noexcept
  {
    return handle.is_valid() && handle_index(handle) < count;
  }

  std::vector<Vertex> m_vertices;
  std::vector<Halfedge> m_halfedges;
  std::vector<Face> m_faces;
  std::vector<Edge> m_edges;
  std::unordered_map<DirectedEdgeKey, HalfedgeHandle, DirectedEdgeKeyHash> m_directedEdges;
  std::unordered_set<FaceKey, FaceKeyHash> m_faceKeys;
};

template <typename T>
using TriangleHalfedgeMesh2 = TriangleHalfedgeMesh<T, 2u>;
template <typename T>
using TriangleHalfedgeMesh3 = TriangleHalfedgeMesh<T, 3u>;

using TriangleHalfedgeMesh2f = TriangleHalfedgeMesh2<float>;
using TriangleHalfedgeMesh3f = TriangleHalfedgeMesh3<float>;

using TriangleHalfedgeMesh2d = TriangleHalfedgeMesh2<double>;
using TriangleHalfedgeMesh3d = TriangleHalfedgeMesh3<double>;

namespace detail
{
using Mesh = TriangleHalfedgeMesh3d;

static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstFaceHalfedgeCirculator>, Mesh::ConstFaceHalfedgeCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::FaceHalfedgeCirculator>, Mesh::FaceHalfedgeCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstFaceVertexCirculator>, Mesh::ConstFaceVertexCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::FaceVertexCirculator>, Mesh::FaceVertexCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstFaceFaceCirculator>, Mesh::ConstFaceFaceCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::FaceFaceCirculator>, Mesh::FaceFaceCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstVertexOutHalfedgeCirculator>, Mesh::ConstVertexOutHalfedgeCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::VertexOutHalfedgeCirculator>, Mesh::VertexOutHalfedgeCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstVertexInHalfedgeCirculator>, Mesh::ConstVertexInHalfedgeCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::VertexInHalfedgeCirculator>, Mesh::VertexInHalfedgeCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstVertexVertexCirculator>, Mesh::ConstVertexVertexCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::VertexVertexCirculator>, Mesh::VertexVertexCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::ConstVertexFaceCirculator>, Mesh::ConstVertexFaceCirculator>);
static_assert(std::sentinel_for<Mesh::FaceCirculatorSentinel<Mesh::VertexFaceCirculator>, Mesh::VertexFaceCirculator>);

static_assert(MeshCirculator<Mesh::ConstFaceHalfedgeCirculator>);
static_assert(MeshCirculator<Mesh::FaceHalfedgeCirculator>);
static_assert(MeshCirculator<Mesh::ConstFaceVertexCirculator>);
static_assert(MeshCirculator<Mesh::FaceVertexCirculator>);
static_assert(MeshCirculator<Mesh::ConstFaceFaceCirculator>);
static_assert(MeshCirculator<Mesh::FaceFaceCirculator>);
static_assert(MeshCirculator<Mesh::ConstVertexOutHalfedgeCirculator>);
static_assert(MeshCirculator<Mesh::VertexOutHalfedgeCirculator>);
static_assert(MeshCirculator<Mesh::ConstVertexInHalfedgeCirculator>);
static_assert(MeshCirculator<Mesh::VertexInHalfedgeCirculator>);
static_assert(MeshCirculator<Mesh::ConstVertexVertexCirculator>);
static_assert(MeshCirculator<Mesh::VertexVertexCirculator>);
static_assert(MeshCirculator<Mesh::ConstVertexFaceCirculator>);
static_assert(MeshCirculator<Mesh::VertexFaceCirculator>);

// Non-const -> const converts; const -> non-const does not.
static_assert(std::is_convertible_v<Mesh::FaceHalfedgeCirculator, Mesh::ConstFaceHalfedgeCirculator>);
static_assert(!std::is_convertible_v<Mesh::ConstFaceHalfedgeCirculator, Mesh::FaceHalfedgeCirculator>);
static_assert(std::is_convertible_v<Mesh::VertexOutHalfedgeCirculator, Mesh::ConstVertexOutHalfedgeCirculator>);
static_assert(!std::is_convertible_v<Mesh::ConstVertexOutHalfedgeCirculator, Mesh::VertexOutHalfedgeCirculator>);
static_assert(std::is_convertible_v<Mesh::VertexInHalfedgeCirculator, Mesh::ConstVertexInHalfedgeCirculator>);
static_assert(!std::is_convertible_v<Mesh::ConstVertexInHalfedgeCirculator, Mesh::VertexInHalfedgeCirculator>);
static_assert(std::is_convertible_v<Mesh::VertexVertexCirculator, Mesh::ConstVertexVertexCirculator>);
static_assert(!std::is_convertible_v<Mesh::ConstVertexVertexCirculator, Mesh::VertexVertexCirculator>);
static_assert(std::is_convertible_v<Mesh::VertexFaceCirculator, Mesh::ConstVertexFaceCirculator>);
static_assert(!std::is_convertible_v<Mesh::ConstVertexFaceCirculator, Mesh::VertexFaceCirculator>);

// Whole-mesh element iterators model the standard iterator concepts, so range-for and std algorithms work.
static_assert(std::forward_iterator<Mesh::VertexIterator>);
static_assert(std::forward_iterator<Mesh::HalfedgeIterator>);
static_assert(std::forward_iterator<Mesh::EdgeIterator>);
static_assert(std::forward_iterator<Mesh::FaceIterator>);
static_assert(std::random_access_iterator<Mesh::VertexIterator>);
static_assert(std::random_access_iterator<Mesh::FaceIterator>);
} // namespace detail

} // namespace Geometry

#endif // GEOMETRY_MESH_TRIANGLEHALFEDGEMESH_HPP
