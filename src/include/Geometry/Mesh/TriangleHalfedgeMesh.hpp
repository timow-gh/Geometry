#ifndef GEOMETRY_MESH_TRIANGLEHALFEDGEMESH_HPP
#define GEOMETRY_MESH_TRIANGLEHALFEDGEMESH_HPP

#include "Geometry/Handle.hpp"
#include "Geometry/Mesh/MeshConnectivity.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
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
    // The stored vertex is the target vertex. The face is on the left when walking source -> target.
    VertexHandle vertex{};
    HalfedgeHandle twin{};
    HalfedgeHandle next{};
    HalfedgeHandle prev{};
    FaceHandle face{};
    EdgeHandle edge{};
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

  template <typename Mesh, bool Const>
  friend class MeshConnectivityView;

  using ConnectivityView = MeshConnectivityView<TriangleHalfedgeMesh, false>;
  using ConstConnectivityView = MeshConnectivityView<TriangleHalfedgeMesh, true>;

  // Explicit, opt-in accessor for the low-level connectivity kernel. Write algorithms
  // obtain the unchecked mutation primitives through this handle
  GEO_NODISCARD ConnectivityView connectivity() noexcept { return ConnectivityView{this}; }
  GEO_NODISCARD ConstConnectivityView connectivity() const noexcept { return ConstConnectivityView{this}; }

  template <bool Const>
  using MeshPtrT = std::conditional_t<Const, const TriangleHalfedgeMesh*, TriangleHalfedgeMesh*>;

  template <bool Const>
  class FaceHalfedgeCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class FaceHalfedgeCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

    public:
      using value_type = Halfedge;
      using reference = std::conditional_t<Const, const Halfedge&, Halfedge&>;
      using pointer = std::conditional_t<Const, const Halfedge*, Halfedge*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr FaceHalfedgeCirculatorT() noexcept = default;
      constexpr explicit FaceHalfedgeCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      // Non-const -> const conversion only. Templated on the source constness with a Const && !Other
      // constraint so this never matches the copy/move constructor of the non-const specialization
      // (which would otherwise suppress the implicit copy/move ctors and break std::semiregular).
      template <bool Other>
      constexpr FaceHalfedgeCirculatorT(const FaceHalfedgeCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_halfedge(m_heHandle); }
      constexpr reference operator*() const noexcept { return m_mesh->get_halfedge(m_heHandle); }

      constexpr FaceHalfedgeCirculatorT& operator++() {
        auto nextHe = m_mesh->get_halfedge(m_heHandle).next;
        assert(nextHe.is_valid());
        m_heHandle = nextHe;
        m_started = true;
        return *this;
      }

      constexpr FaceHalfedgeCirculatorT operator++(int) {
        FaceHalfedgeCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const FaceHalfedgeCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const FaceHalfedgeCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstFaceHalfedgeCirculator = FaceHalfedgeCirculatorT<true>;
  using FaceHalfedgeCirculator = FaceHalfedgeCirculatorT<false>;

  template <bool Const>
  class FaceVertexCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class FaceVertexCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

    public:
      using value_type = Vertex;
      using reference = std::conditional_t<Const, const Vertex&, Vertex&>;
      using pointer = std::conditional_t<Const, const Vertex*, Vertex*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr FaceVertexCirculatorT() noexcept = default;
      constexpr explicit FaceVertexCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      template <bool Other>
      constexpr FaceVertexCirculatorT(const FaceVertexCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_vertex(m_mesh->target_vertex(m_heHandle)); }
      constexpr reference operator*() const noexcept { return m_mesh->get_vertex(m_mesh->target_vertex(m_heHandle)); }

      constexpr FaceVertexCirculatorT& operator++() {
        auto nextHe = m_mesh->get_halfedge(m_heHandle).next;
        assert(nextHe.is_valid());
        m_heHandle = nextHe;
        m_started = true;
        return *this;
      }

      constexpr FaceVertexCirculatorT operator++(int) {
        FaceVertexCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const FaceVertexCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const FaceVertexCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr VertexHandle get_vertexhandle() const noexcept { return m_mesh->target_vertex(m_heHandle); }
      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstFaceVertexCirculator = FaceVertexCirculatorT<true>;
  using FaceVertexCirculator = FaceVertexCirculatorT<false>;

  template <bool Const>
  class FaceFaceCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class FaceFaceCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

      [[nodiscard]] constexpr bool cursor_is_boundary() const noexcept { return !m_mesh->get_halfedge(m_heHandle).twin.is_valid(); }

      [[nodiscard]] constexpr FaceHandle neighbor_face() const noexcept {
        return m_mesh->get_halfedge(m_mesh->get_halfedge(m_heHandle).twin).face;
      }

      constexpr void skip_boundary() noexcept {
        while (m_heHandle.is_valid() && !has_returned_to_start() && cursor_is_boundary())
        {
          m_heHandle = m_mesh->get_halfedge(m_heHandle).next;
          m_started = true;
        }
      }

    public:
      using value_type = Face;
      using reference = std::conditional_t<Const, const Face&, Face&>;
      using pointer = std::conditional_t<Const, const Face*, Face*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr FaceFaceCirculatorT() noexcept = default;
      constexpr explicit FaceFaceCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {
        if (m_mesh != nullptr)
        {
          skip_boundary();
        }
      }

      template <bool Other>
      constexpr FaceFaceCirculatorT(const FaceFaceCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_face(neighbor_face()); }
      constexpr reference operator*() const noexcept { return m_mesh->get_face(neighbor_face()); }

      constexpr FaceFaceCirculatorT& operator++() {
        auto nextHe = m_mesh->get_halfedge(m_heHandle).next;
        assert(nextHe.is_valid());
        m_heHandle = nextHe;
        m_started = true;
        skip_boundary();
        return *this;
      }

      constexpr FaceFaceCirculatorT operator++(int) {
        FaceFaceCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const FaceFaceCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const FaceFaceCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr FaceHandle get_facehandle() const noexcept { return neighbor_face(); }
      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstFaceFaceCirculator = FaceFaceCirculatorT<true>;
  using FaceFaceCirculator = FaceFaceCirculatorT<false>;

  template <bool Const>
  class VertexOutHalfedgeCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class VertexOutHalfedgeCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

    public:
      using value_type = Halfedge;
      using reference = std::conditional_t<Const, const Halfedge&, Halfedge&>;
      using pointer = std::conditional_t<Const, const Halfedge*, Halfedge*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr VertexOutHalfedgeCirculatorT() noexcept = default;
      constexpr explicit VertexOutHalfedgeCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      template <bool Other>
      constexpr VertexOutHalfedgeCirculatorT(const VertexOutHalfedgeCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_halfedge(m_heHandle); }
      constexpr reference operator*() const noexcept { return m_mesh->get_halfedge(m_heHandle); }

      constexpr VertexOutHalfedgeCirculatorT& operator++() {
        HalfedgeHandle const twinHandle = m_mesh->get_halfedge(m_heHandle).twin;
        m_started = true;
        if (!twinHandle.is_valid())
        {
          m_heHandle = HalfedgeHandle{};
          return *this;
        }
        m_heHandle = m_mesh->get_halfedge(twinHandle).next;
        return *this;
      }

      constexpr VertexOutHalfedgeCirculatorT operator++(int) {
        VertexOutHalfedgeCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const VertexOutHalfedgeCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const VertexOutHalfedgeCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstVertexOutHalfedgeCirculator = VertexOutHalfedgeCirculatorT<true>;
  using VertexOutHalfedgeCirculator = VertexOutHalfedgeCirculatorT<false>;

  template <bool Const>
  class VertexInHalfedgeCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class VertexInHalfedgeCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

      [[nodiscard]] constexpr HalfedgeHandle incoming_halfedge() const noexcept {
        return m_mesh->get_halfedge(m_heHandle).prev;
      }

    public:
      using value_type = Halfedge;
      using reference = std::conditional_t<Const, const Halfedge&, Halfedge&>;
      using pointer = std::conditional_t<Const, const Halfedge*, Halfedge*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr VertexInHalfedgeCirculatorT() noexcept = default;
      constexpr explicit VertexInHalfedgeCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      template <bool Other>
      constexpr VertexInHalfedgeCirculatorT(const VertexInHalfedgeCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_halfedge(incoming_halfedge()); }
      constexpr reference operator*() const noexcept { return m_mesh->get_halfedge(incoming_halfedge()); }

      constexpr VertexInHalfedgeCirculatorT& operator++() {
        HalfedgeHandle const twinHandle = m_mesh->get_halfedge(m_heHandle).twin;
        m_started = true;
        if (!twinHandle.is_valid())
        {
          m_heHandle = HalfedgeHandle{};
          return *this;
        }
        m_heHandle = m_mesh->get_halfedge(twinHandle).next;
        return *this;
      }

      constexpr VertexInHalfedgeCirculatorT operator++(int) {
        VertexInHalfedgeCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const VertexInHalfedgeCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const VertexInHalfedgeCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr HalfedgeHandle get_halfedgehandle() const noexcept { return incoming_halfedge(); }
      [[nodiscard]] constexpr const HalfedgeHandle& get_outgoinghalfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstVertexInHalfedgeCirculator = VertexInHalfedgeCirculatorT<true>;
  using VertexInHalfedgeCirculator = VertexInHalfedgeCirculatorT<false>;

  template <bool Const>
  class VertexVertexCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class VertexVertexCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

    public:
      using value_type = Vertex;
      using reference = std::conditional_t<Const, const Vertex&, Vertex&>;
      using pointer = std::conditional_t<Const, const Vertex*, Vertex*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr VertexVertexCirculatorT() noexcept = default;
      constexpr explicit VertexVertexCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      template <bool Other>
      constexpr VertexVertexCirculatorT(const VertexVertexCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_vertex(m_mesh->target_vertex(m_heHandle)); }
      constexpr reference operator*() const noexcept { return m_mesh->get_vertex(m_mesh->target_vertex(m_heHandle)); }

      constexpr VertexVertexCirculatorT& operator++() {
        HalfedgeHandle const twinHandle = m_mesh->get_halfedge(m_heHandle).twin;
        m_started = true;
        if (!twinHandle.is_valid())
        {
          m_heHandle = HalfedgeHandle{};
          return *this;
        }
        m_heHandle = m_mesh->get_halfedge(twinHandle).next;
        return *this;
      }

      constexpr VertexVertexCirculatorT operator++(int) {
        VertexVertexCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const VertexVertexCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const VertexVertexCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr VertexHandle get_vertexhandle() const noexcept { return m_mesh->target_vertex(m_heHandle); }
      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstVertexVertexCirculator = VertexVertexCirculatorT<true>;
  using VertexVertexCirculator = VertexVertexCirculatorT<false>;

  template <bool Const>
  class VertexFaceCirculatorT
  {
      MeshPtrT<Const> m_mesh{nullptr};
      HalfedgeHandle m_heHandle{};
      HalfedgeHandle m_start{};
      bool m_started{false};

      template <bool> friend class VertexFaceCirculatorT;

      [[nodiscard]] constexpr bool has_returned_to_start() const noexcept { return m_started && m_heHandle == m_start; }

      [[nodiscard]] constexpr FaceHandle incident_face() const noexcept { return m_mesh->get_halfedge(m_heHandle).face; }

    public:
      using value_type = Face;
      using reference = std::conditional_t<Const, const Face&, Face&>;
      using pointer = std::conditional_t<Const, const Face*, Face*>;
      using mesh_pointer = MeshPtrT<Const>;
      using size_type = std::ptrdiff_t;
      using difference_type = std::ptrdiff_t;

      constexpr VertexFaceCirculatorT() noexcept = default;
      constexpr explicit VertexFaceCirculatorT(HalfedgeHandle heHandle, MeshPtrT<Const> mesh) noexcept
          : m_mesh(mesh)
          , m_heHandle(heHandle)
          , m_start(heHandle) {}

      template <bool Other>
      constexpr VertexFaceCirculatorT(const VertexFaceCirculatorT<Other>& other) noexcept
          requires(Const && !Other)
          : m_mesh(other.m_mesh)
          , m_heHandle(other.m_heHandle)
          , m_start(other.m_start)
          , m_started(other.m_started) {}

      constexpr pointer operator->() const noexcept { return &m_mesh->get_face(incident_face()); }
      constexpr reference operator*() const noexcept { return m_mesh->get_face(incident_face()); }

      constexpr VertexFaceCirculatorT& operator++() {
        HalfedgeHandle const twinHandle = m_mesh->get_halfedge(m_heHandle).twin;
        m_started = true;
        if (!twinHandle.is_valid())
        {
          m_heHandle = HalfedgeHandle{};
          return *this;
        }
        m_heHandle = m_mesh->get_halfedge(twinHandle).next;
        return *this;
      }

      constexpr VertexFaceCirculatorT operator++(int) {
        VertexFaceCirculatorT old = *this;
        operator++();
        return old;
      }

      [[nodiscard]] constexpr bool is_valid() const noexcept {
        return m_heHandle.is_valid() && !has_returned_to_start();
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_valid(); }

      [[nodiscard]] constexpr bool operator==(const VertexFaceCirculatorT& other) const noexcept {
        return m_mesh == other.m_mesh &&
                m_heHandle == other.m_heHandle &&
                m_start == other.m_start &&
                m_started == other.m_started;
      }
      [[nodiscard]] constexpr bool operator!=(const VertexFaceCirculatorT& other) const noexcept { return !(*this == other); }

      [[nodiscard]] constexpr FaceHandle get_facehandle() const noexcept { return incident_face(); }
      [[nodiscard]] constexpr const HalfedgeHandle& get_halfedgehandle() const noexcept { return m_heHandle; }
      [[nodiscard]] constexpr const HalfedgeHandle& get_starthandle() const noexcept { return m_start; }
  };

  using ConstVertexFaceCirculator = VertexFaceCirculatorT<true>;
  using VertexFaceCirculator = VertexFaceCirculatorT<false>;

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

  using FaceHalfedgeRange = FaceCirculatorRange<ConstFaceHalfedgeCirculator>;
  using FaceHalfedgeRangeMutable = FaceCirculatorRange<FaceHalfedgeCirculator>;
  using FaceVertexRange = FaceCirculatorRange<ConstFaceVertexCirculator>;
  using FaceVertexRangeMutable = FaceCirculatorRange<FaceVertexCirculator>;
  using FaceFaceRange = FaceCirculatorRange<ConstFaceFaceCirculator>;
  using FaceFaceRangeMutable = FaceCirculatorRange<FaceFaceCirculator>;

  using VertexOutHalfedgeRange = FaceCirculatorRange<ConstVertexOutHalfedgeCirculator>;
  using VertexOutHalfedgeRangeMutable = FaceCirculatorRange<VertexOutHalfedgeCirculator>;
  using VertexInHalfedgeRange = FaceCirculatorRange<ConstVertexInHalfedgeCirculator>;
  using VertexInHalfedgeRangeMutable = FaceCirculatorRange<VertexInHalfedgeCirculator>;
  using VertexVertexRange = FaceCirculatorRange<ConstVertexVertexCirculator>;
  using VertexVertexRangeMutable = FaceCirculatorRange<VertexVertexCirculator>;
  using VertexFaceRange = FaceCirculatorRange<ConstVertexFaceCirculator>;
  using VertexFaceRangeMutable = FaceCirculatorRange<VertexFaceCirculator>;

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
    VertexHandle const handle = make_handle<VertexHandle>(m_vertices.size());
    m_vertices.reserve(m_vertices.size() + 1);
    m_vertexHalfedges.reserve(m_vertexHalfedges.size() + 1);
    m_vertices.push_back(Vertex{position, HalfedgeHandle{}});
    m_vertexHalfedges.emplace_back();
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

  GEO_NODISCARD FaceHalfedgeRange halfedges(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceHalfedgeRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceHalfedgeRangeMutable halfedges(FaceHandle face) noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceHalfedgeRangeMutable(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceVertexRange vertices(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceVertexRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceVertexRangeMutable vertices(FaceHandle face) noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceVertexRangeMutable(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceFaceRange adjacent_faces(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceFaceRange(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD FaceFaceRangeMutable adjacent_faces(FaceHandle face) noexcept
  {
    GEO_ASSERT(contains(face));
    return FaceFaceRangeMutable(get_face(face).get_halfedgehandle(), this);
  }

  GEO_NODISCARD VertexOutHalfedgeRange outgoing_halfedges(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexOutHalfedgeRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexOutHalfedgeRangeMutable outgoing_halfedges(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexOutHalfedgeRangeMutable(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexInHalfedgeRange incoming_halfedges(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexInHalfedgeRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexInHalfedgeRangeMutable incoming_halfedges(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexInHalfedgeRangeMutable(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexVertexRange vertices(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexVertexRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexVertexRangeMutable vertices(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexVertexRangeMutable(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexFaceRange faces(VertexHandle vertex) const noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexFaceRange(get_vertex(vertex).halfedge, this);
  }

  GEO_NODISCARD VertexFaceRangeMutable faces(VertexHandle vertex) noexcept
  {
    GEO_ASSERT(contains(vertex));
    return VertexFaceRangeMutable(get_vertex(vertex).halfedge, this);
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
    std::array<HalfedgeHandle, 3> const halfedges = halfedges_around_face(face);
    return {source_vertex(halfedges[0]), target_vertex(halfedges[0]), target_vertex(halfedges[1])};
  }

  GEO_NODISCARD std::vector<HalfedgeHandle> halfedges_around_vertex(VertexHandle vertex) const
  {
    GEO_ASSERT(contains(vertex));

    std::vector<HalfedgeHandle> result;
    std::vector<unsigned char> visited(m_halfedges.size(), 0U);
    append_halfedges_around_vertex_fan(vertex, get_vertex(vertex).halfedge, result, visited);

    for (HalfedgeHandle const halfedge : m_vertexHalfedges[handle_index(vertex)])
    {
      append_halfedges_around_vertex_fan(vertex, halfedge, result, visited);
    }

    return result;
  }

  GEO_NODISCARD std::vector<FaceHandle> faces_around_vertex(VertexHandle vertex) const
  {
    std::vector<HalfedgeHandle> const halfedges = halfedges_around_vertex(vertex);
    std::vector<FaceHandle> result;
    result.reserve(halfedges.size());

    for (HalfedgeHandle halfedge : halfedges)
    {
      result.push_back(get_halfedge(halfedge).face);
    }

    return result;
  }

  GEO_NODISCARD VertexHandle source_vertex(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    HalfedgeHandle const prev = get_halfedge(halfedge).prev;
    GEO_ASSERT(contains(prev));
    return get_halfedge(prev).vertex;
  }

  GEO_NODISCARD VertexHandle target_vertex(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    return get_halfedge(halfedge).vertex;
  }

  GEO_NODISCARD bool is_boundary(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    return !get_halfedge(halfedge).twin.is_valid();
  }

  GEO_NODISCARD bool is_boundary(EdgeHandle edge) const noexcept
  {
    GEO_ASSERT(contains(edge));
    return is_boundary(get_edge(edge).halfedge);
  }

  GEO_NODISCARD bool is_valid() const noexcept
  {
    if (m_vertexHalfedges.size() != m_vertices.size())
    {
      return false;
    }

    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      const Halfedge& halfedge = m_halfedges[i];

      if (!contains(halfedge.vertex) || !contains(halfedge.next) || !contains(halfedge.prev) || !contains(halfedge.face)
          || !contains(halfedge.edge))
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_vertices.size(); ++i)
    {
      VertexHandle const vertex = make_handle<VertexHandle>(i);
      HalfedgeHandle const halfedge = m_vertices[i].halfedge;
      if (halfedge.is_valid() && (!contains(halfedge) || unchecked_source_vertex(halfedge) != vertex))
      {
        return false;
      }

      for (HalfedgeHandle const outgoingHalfedge : m_vertexHalfedges[i])
      {
        if (!contains(outgoingHalfedge) || unchecked_source_vertex(outgoingHalfedge) != vertex)
        {
          return false;
        }
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      FaceHandle const face = make_handle<FaceHandle>(i);
      HalfedgeHandle const firstHalfedge = m_faces[i].get_halfedgehandle();
      if (!contains(firstHalfedge))
      {
        return false;
      }

      HalfedgeHandle const secondHalfedge = m_halfedges[handle_index(firstHalfedge)].next;
      HalfedgeHandle const thirdHalfedge = m_halfedges[handle_index(secondHalfedge)].next;
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
      EdgeHandle const edge = make_handle<EdgeHandle>(i);
      HalfedgeHandle const halfedge = m_edges[i].halfedge;
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
      HalfedgeHandle const halfedgeHandle = make_handle<HalfedgeHandle>(i);
      const Halfedge& halfedge = m_halfedges[i];

      if (m_halfedges[handle_index(halfedge.next)].prev != halfedgeHandle
          || m_halfedges[handle_index(halfedge.prev)].next != halfedgeHandle)
      {
        return false;
      }

      if (halfedge.twin.is_valid())
      {
        if (!contains(halfedge.twin))
        {
          return false;
        }

        const Halfedge& twin = m_halfedges[handle_index(halfedge.twin)];
        if (twin.twin != halfedgeHandle || twin.edge != halfedge.edge
            || unchecked_source_vertex(halfedgeHandle) != unchecked_target_vertex(halfedge.twin)
            || unchecked_target_vertex(halfedgeHandle) != unchecked_source_vertex(halfedge.twin))
        {
          return false;
        }
      }

      DirectedEdgeKey const key{unchecked_source_vertex(halfedgeHandle).get_value(), unchecked_target_vertex(halfedgeHandle).get_value()};
      auto const directedEdgeIt = m_directedEdges.find(key);
      if (directedEdgeIt == m_directedEdges.end() || directedEdgeIt->second != halfedgeHandle)
      {
        return false;
      }

      VertexHandle const source = unchecked_source_vertex(halfedgeHandle);
      bool foundInVertexHalfedges = false;
      for (HalfedgeHandle const outgoingHalfedge : m_vertexHalfedges[handle_index(source)])
      {
        if (outgoingHalfedge == halfedgeHandle)
        {
          foundInVertexHalfedges = true;
          break;
        }
      }

      if (!foundInVertexHalfedges)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      FaceHandle const face = make_handle<FaceHandle>(i);
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
      size_type const from = static_cast<size_type>(key.from);
      size_type const to = static_cast<size_type>(key.to);
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
        size_type const value = static_cast<size_type>(vertex);
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

  void append_halfedges_around_vertex_fan(VertexHandle vertex,
                                          HalfedgeHandle start,
                                          std::vector<HalfedgeHandle>& result,
                                          std::vector<unsigned char>& visited) const
  {
    if (!contains(start) || visited[handle_index(start)] != 0U || source_vertex(start) != vertex)
    {
      return;
    }

    HalfedgeHandle first = start;
    while (true)
    {
      HalfedgeHandle const previous = get_halfedge(first).prev;
      HalfedgeHandle const opposite = get_halfedge(previous).twin;
      if (!opposite.is_valid() || !contains(opposite) || source_vertex(opposite) != vertex || opposite == start
          || visited[handle_index(opposite)] != 0U)
      {
        break;
      }

      first = opposite;
    }

    HalfedgeHandle current = first;
    while (contains(current) && visited[handle_index(current)] == 0U && source_vertex(current) == vertex)
    {
      result.push_back(current);
      visited[handle_index(current)] = 1U;

      HalfedgeHandle const twin = get_halfedge(current).twin;
      if (!twin.is_valid() || !contains(twin))
      {
        break;
      }

      current = get_halfedge(twin).next;
    }
  }

  GEO_NODISCARD VertexHandle unchecked_source_vertex(HalfedgeHandle halfedge) const noexcept
  {
    return m_halfedges[handle_index(m_halfedges[handle_index(halfedge)].prev)].vertex;
  }

  GEO_NODISCARD VertexHandle unchecked_target_vertex(HalfedgeHandle halfedge) const noexcept
  {
    return m_halfedges[handle_index(halfedge)].vertex;
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
  std::vector<std::vector<HalfedgeHandle>> m_vertexHalfedges;
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
