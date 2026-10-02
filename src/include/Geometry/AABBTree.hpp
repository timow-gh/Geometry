#ifndef GEOMETRY_AABBTREE_HPP
#define GEOMETRY_AABBTREE_HPP

#include "Geometry/AABB.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace Geometry
{

namespace detail
{

/**
 * \internal
 * \brief Whether \p box has min <= max on every axis; flat (zero-extent) boxes are allowed.
 *
 * \c AABB::is_valid demands min < max with a tolerance and therefore rejects the box of every
 * axis-aligned triangle. NaN coordinates fail the comparison and make the box invalid.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr bool has_ordered_bounds(const AABB<T, 3>& box) noexcept
{
  using Vec3 = linal::vec3<T>;

  const Vec3 min = box.get_min();
  const Vec3 max = box.get_max();
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    if (!(min[axis] <= max[axis]))
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Whether every coordinate of \p box is finite, i.e. neither infinite nor NaN.
 *
 * The build orders boxes by their centers, and an axis spanning (-inf, +inf) or a corner at
 * infinity has a NaN center. A NaN key breaks the strict weak ordering \c std::nth_element relies
 * on, so such boxes must be rejected before the build. Range comparisons instead of
 * \c std::isfinite keep this usable in constant expressions; NaN fails them as well.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr bool has_finite_coordinates(const AABB<T, 3>& box) noexcept
{
  using Vec3 = linal::vec3<T>;

  constexpr T lowest = std::numeric_limits<T>::lowest();
  constexpr T highest = std::numeric_limits<T>::max();
  const Vec3 min = box.get_min();
  const Vec3 max = box.get_max();
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    if (!(lowest <= min[axis] && min[axis] <= highest && lowest <= max[axis] && max[axis] <= highest))
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Twice the center of \p box along \p axis.
 *
 * Orders boxes by center like \c AABB::center, but reads one axis and skips the halving, which the
 * build's split runs for every comparison. For finite boxes the key is never NaN; a sum that
 * overflows becomes an infinity, which still orders correctly.
 *
 * \pre axis < 3, and every coordinate of \p box is finite.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr T center_key(const AABB<T, 3>& box, const std::uint8_t axis) noexcept
{
  GEO_ASSERT(axis < 3);
  GEO_ASSERT(has_finite_coordinates(box));
  return box.get_min()[axis] + box.get_max()[axis];
}

/**
 * \internal
 * \brief Box with the corners \p min and \p max, which may coincide on any axis.
 *
 * The (min, max) constructor of \c AABB asserts a non-zero extent, but flat boxes are valid input
 * to the tree, so the corners are set directly.
 *
 * \pre min <= max on every axis.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr AABB<T, 3> make_closed_box(const linal::vec3<T>& min, const linal::vec3<T>& max) noexcept
{
  AABB<T, 3> box;
  box.set_min(min);
  box.set_max(max);
  GEO_ASSERT(has_ordered_bounds(box));
  return box;
}

/**
 * \internal
 * \brief Whether the closed boxes \p lhs and \p rhs share at least one point; touching counts.
 *
 * \c is_intersecting(AABB, AABB) is open and compares with a tolerance, so it drops touching and
 * barely overlapping boxes. The broad phase of the mesh Booleans must never drop a pair whose
 * triangles meet: coplanar contact only produces touching boxes. A false positive costs one
 * narrow-phase test, a false negative leaves a gap in the result, hence exact comparisons.
 *
 * \pre \p lhs and \p rhs have min <= max on every axis; for reversed or NaN bounds the answer is
 * meaningless.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr bool is_overlapping_closed(const AABB<T, 3>& lhs, const AABB<T, 3>& rhs) noexcept
{
  using Vec3 = linal::vec3<T>;

  GEO_ASSERT(has_ordered_bounds(lhs));
  GEO_ASSERT(has_ordered_bounds(rhs));

  const Vec3 lhsMin = lhs.get_min();
  const Vec3 lhsMax = lhs.get_max();
  const Vec3 rhsMin = rhs.get_min();
  const Vec3 rhsMax = rhs.get_max();
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    if (lhsMax[axis] < rhsMin[axis] || rhsMax[axis] < lhsMin[axis])
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Whether the closed box \p outer contains every point of the closed box \p inner.
 *
 * The invariant both traversals rest on: a node's box encloses its children's, so skipping a node
 * that misses the query never skips a box that hits it. Exact comparisons, for the reason given at
 * \c is_overlapping_closed.
 *
 * \pre \p outer and \p inner have min <= max on every axis.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr bool contains(const AABB<T, 3>& outer, const AABB<T, 3>& inner) noexcept
{
  using Vec3 = linal::vec3<T>;

  GEO_ASSERT(has_ordered_bounds(outer));
  GEO_ASSERT(has_ordered_bounds(inner));

  const Vec3 outerMin = outer.get_min();
  const Vec3 outerMax = outer.get_max();
  const Vec3 innerMin = inner.get_min();
  const Vec3 innerMax = inner.get_max();
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    if (innerMin[axis] < outerMin[axis] || outerMax[axis] < innerMax[axis])
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Smallest box containing both \p lhs and \p rhs.
 *
 * Computed with exact min/max instead of \c AABB::add: \c add ignores growth below its tolerance, so
 * a child could stick out of its parent and the traversal would skip an overlapping pair.
 *
 * \pre \p lhs and \p rhs have min <= max on every axis.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr AABB<T, 3> enclosing_box(const AABB<T, 3>& lhs, const AABB<T, 3>& rhs) noexcept
{
  using Vec3 = linal::vec3<T>;

  GEO_ASSERT(has_ordered_bounds(lhs));
  GEO_ASSERT(has_ordered_bounds(rhs));

  const Vec3 lhsMin = lhs.get_min();
  const Vec3 lhsMax = lhs.get_max();
  const Vec3 rhsMin = rhs.get_min();
  const Vec3 rhsMax = rhs.get_max();
  Vec3 min;
  Vec3 max;
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    min[axis] = std::min(lhsMin[axis], rhsMin[axis]);
    max[axis] = std::max(lhsMax[axis], rhsMax[axis]);
  }
  return make_closed_box(min, max);
}

/**
 * \internal
 * \brief Sum of the extents of \p box along the three axes (half its perimeter).
 *
 * A cheap size measure: the pair traversal descends into the larger of two boxes, which shrinks the
 * overlap fastest.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr T extent_sum(const AABB<T, 3>& box) noexcept
{
  using Vec3 = linal::vec3<T>;

  const Vec3 extent{box.get_max() - box.get_min()};
  return extent[0] + extent[1] + extent[2];
}

/**
 * \internal
 * \brief Stack with a fixed capacity, so tree traversals never allocate.
 *
 * The capacity is a proven bound derived from the tree depth; exceeding it is a logic error caught
 * by an assertion, not a runtime condition.
 */
template <typename TValue, std::size_t Capacity>
class FixedStack {
  std::array<TValue, Capacity> m_values{};
  std::size_t m_size{0};

public:
  GEO_NODISCARD constexpr bool empty() const noexcept { return m_size == 0; }

  constexpr void push(const TValue& value) noexcept
  {
    GEO_ASSERT(m_size < Capacity);
    m_values[m_size++] = value;
  }

  GEO_NODISCARD constexpr TValue pop() noexcept
  {
    GEO_ASSERT(m_size > 0);
    return m_values[--m_size];
  }
};

} // namespace detail

/** \brief Reason \c AABBTree::create_from_boxes could not build a tree; Ok on success. */
enum class AABBTreeStatus
{
  Ok,
  // More boxes than the tree's index type can number.
  IndexCapacityExceeded,
  // A box has min > max or a non-finite (infinite or NaN) coordinate on some axis.
  InvalidBounds
};

template <typename T, typename TIndex>
struct AABBTreeResult;

/**
 * \brief Static bounding volume hierarchy over 3D boxes; finds the boxes that overlap a query box,
 * or the overlapping box pairs of two trees (see \c for_each_overlapping and
 * \c for_each_overlapping_pair).
 *
 * Built once and never updated: the mesh Booleans query the face boxes of meshes that do not change
 * during the broad phase. Queries report a box by its index in the span given to
 * \c create_from_boxes, so callers map results back to faces themselves.
 *
 * Overlap is closed and exact (\c detail::is_overlapping_closed): touching and flat boxes are
 * reported, because the broad phase must never miss a contact.
 *
 * Nodes live in one flat vector in depth-first pre-order, the left child directly after its parent, so
 * building costs two allocations and traversal walks contiguous memory. The tree is built top down
 * by median split, which halves the box count per level and bounds the depth by
 * log2(size) independent of the geometry; traversals rely on that bound for their fixed stacks.
 *
 * \tparam T coordinate type.
 * \tparam TIndex unsigned type of the reported box indices.
 */
template <typename T, typename TIndex = std::uint32_t>
class AABBTree {
  static_assert(std::is_unsigned_v<TIndex>, "box indices must be unsigned");

public:
  using Box = AABB<T, 3>;
  using index_type = TIndex;

  /**
   * \brief Small leaves trade a few extra box tests for fewer nodes. At least 3 so that splitting a
   * node gives each child at least two boxes, which bounds the node count by the box count.
   */
  static constexpr std::size_t max_leaf_size = 4;
  static_assert(max_leaf_size >= 3);

  /**
   * \brief Upper bound on node depth (root at 0). Sizes the traversal stacks; it is not a build limit.
   *
   * The build stops splitting at max_leaf_size and never consults this. Median splits halve the box
   * count per level, so a tree over n boxes is about log2(n) deep; n fits in index_type, so the depth
   * stays below its bit count (32 for std::uint32_t).
   */
  static constexpr std::size_t max_depth = std::numeric_limits<index_type>::digits;

  /**
   * \brief One box of the input together with its index in the span given to \c create_from_boxes.
   *
   * The build reorders the indexed boxes so that every leaf owns a contiguous range of them.
   */
  struct IndexedBox
  {
    Box box;
    /**
     * \brief Index of the box in the input span, reported to the overlap callbacks so callers can
     * map a hit back to their own data. Not the index into indexed_boxes(), which the build
     * reorders.
     */
    index_type inputIndex{};
  };

  /**
   * \brief A node of the hierarchy: either a leaf that owns indexed boxes, or an inner node with two
   * children.
   *
   * A leaf owns the indexed boxes [first_box_idx(), first_box_idx() + box_count()). An inner node
   * always has two children, but only the right one is stored: nodes are stored in pre-order, each
   * node followed by its whole left subtree and then its whole right subtree. So the left child of
   * node i is always node i + 1, while the right child comes after the left subtree, whose size
   * varies:
   *
   * \verbatim
   *         A0             storage: [A, B, C, D, E, F, G]
   *       /    \
   *     B1      E4         left children:  B = A + 1, C = B + 1, F = E + 1
   *    /  \    /  \        right children: A's is 4, because B's subtree fills 1..3
   *   C2  D3  F5  G6
   * \endverbatim
   *
   * Leaving out the left child keeps nodes small, and descending to the left child reads the next
   * node in memory. Use \c AABBTree::left_child_idx and \c AABBTree::right_child_idx to reach the
   * children.
   *
   * A class rather than a struct because which fields are meaningful depends on the kind of node;
   * the accessors assert that only the meaningful ones are read.
   */
  class Node {
    // Encloses every box of the subtree.
    Box m_bounds;
    // Leaves: index into indexed_boxes() of the leaf's first box. Inner nodes: unused.
    index_type m_firstBoxIdx{};
    // Leaves: number of boxes the leaf owns. Inner nodes: 0, which marks them as inner.
    index_type m_boxCount{};
    // Inner nodes: index into nodes() of the right child. Leaves: unused.
    index_type m_rightChildIdx{};

    constexpr Node(const Box& bounds,
                   const index_type firstBoxIdx,
                   const index_type boxCount,
                   const index_type rightChildIdx) noexcept
        : m_bounds(bounds)
        , m_firstBoxIdx(firstBoxIdx)
        , m_boxCount(boxCount)
        , m_rightChildIdx(rightChildIdx)
    {
      // Node boxes enclose validated input boxes, so they are finite and closed; the traversals and
      // detail::extent_sum rely on that.
      GEO_ASSERT(detail::has_finite_coordinates(bounds));
      GEO_ASSERT(detail::has_ordered_bounds(bounds));
    }

  public:
    /** \brief Leaf owning the indexed boxes [firstBoxIdx, firstBoxIdx + boxCount). \pre boxCount > 0. */
    GEO_NODISCARD static constexpr Node
    create_leaf(const Box& bounds, const index_type firstBoxIdx, const index_type boxCount) noexcept
    {
      GEO_ASSERT(boxCount > 0);
      return Node{bounds, firstBoxIdx, boxCount, index_type{0}};
    }

    /**
     * \brief Inner node whose right child is at index \p rightChildIdx in nodes().
     *
     * \pre rightChildIdx > 0; index 0 is the root, which is no node's child.
     */
    GEO_NODISCARD static constexpr Node create_inner(const Box& bounds, const index_type rightChildIdx) noexcept
    {
      GEO_ASSERT(rightChildIdx > 0);
      return Node{bounds, index_type{0}, index_type{0}, rightChildIdx};
    }

    /** \brief Box enclosing every box of the subtree. */
    GEO_NODISCARD constexpr const Box& bounds() const noexcept { return m_bounds; }

    GEO_NODISCARD constexpr bool is_leaf() const noexcept { return m_boxCount != 0; }

    /** \brief Index into indexed_boxes() of the leaf's first box. \pre is_leaf(). */
    GEO_NODISCARD constexpr index_type first_box_idx() const noexcept
    {
      GEO_ASSERT(is_leaf());
      return m_firstBoxIdx;
    }

    /** \brief Number of boxes the leaf owns. \pre is_leaf(). */
    GEO_NODISCARD constexpr index_type box_count() const noexcept
    {
      GEO_ASSERT(is_leaf());
      return m_boxCount;
    }

    /**
     * \brief Index into nodes() of the inner node's right child. \pre !is_leaf().
     *
     * Traversals use \c AABBTree::right_child_idx instead, next to \c AABBTree::left_child_idx, which a
     * node cannot provide because it does not know its own index.
     */
    GEO_NODISCARD constexpr index_type right_child_idx() const noexcept
    {
      GEO_ASSERT(!is_leaf());
      return m_rightChildIdx;
    }
  };

  /** \brief Empty tree; queries on it report nothing. */
  AABBTree() noexcept = default;

  /**
   * \brief Builds the hierarchy over \p boxes, or reports why it cannot. O(n log n).
   *
   * A named constructor because a constructor can report invalid input only by throwing or
   * asserting; this returns a status the caller can act on. A failure carries an empty tree.
   * Allocation failures are not reported here: they propagate from the containers.
   *
   * Flat (zero-extent) boxes are valid input; boxes with infinite coordinates are not.
   */
  GEO_NODISCARD static AABBTreeResult<T, TIndex> create_from_boxes(const std::span<const Box> boxes)
  {
    using Result = AABBTreeResult<T, TIndex>;

    if (!std::in_range<index_type>(boxes.size()))
    {
      return Result{AABBTree{}, AABBTreeStatus::IndexCapacityExceeded};
    }
    if (!std::ranges::all_of(boxes, [](const Box& box) {
          return detail::has_finite_coordinates(box) && detail::has_ordered_bounds(box);
        }))
    {
      return Result{AABBTree{}, AABBTreeStatus::InvalidBounds};
    }

    AABBTree tree;
    tree.m_indexedBoxes.reserve(boxes.size());
    for (std::size_t i = 0; i < boxes.size(); ++i)
    {
      tree.m_indexedBoxes.push_back(IndexedBox{boxes[i], static_cast<index_type>(i)});
    }
    if (!boxes.empty())
    {
      // Every leaf except a lone root holds at least two boxes (see max_leaf_size), so a binary tree
      // with at most size / 2 leaves has fewer than size nodes.
      tree.m_nodes.reserve(boxes.size());
      tree.build_subtree(0, boxes.size(), 0);
      GEO_ASSERT(boxes.size() == 1 || tree.m_nodes.size() < boxes.size());
    }
    return Result{std::move(tree), AABBTreeStatus::Ok};
  }

  /** \brief Number of boxes in the tree. */
  GEO_NODISCARD std::size_t size() const noexcept { return m_indexedBoxes.size(); }

  GEO_NODISCARD bool empty() const noexcept { return m_indexedBoxes.empty(); }

  /** \brief Nodes in pre-order; the root is the first node. Read-only view for traversals and tests. */
  GEO_NODISCARD std::span<const Node> nodes() const noexcept { return m_nodes; }

  /** \brief Indexed boxes in leaf order. Read-only view for traversals and tests. */
  GEO_NODISCARD std::span<const IndexedBox> indexed_boxes() const noexcept { return m_indexedBoxes; }

  /**
   * \brief Index into nodes() of the left child of the inner node at \p nodeIndex.
   *
   * Not stored but implied by the pre-order storage: the left child directly follows its parent
   * (see \c Node).
   *
   * \pre The node at \p nodeIndex is an inner node.
   * O(1).
   */
  GEO_NODISCARD index_type left_child_idx(const index_type nodeIndex) const noexcept
  {
    GEO_ASSERT(nodeIndex < m_nodes.size());
    GEO_ASSERT(!m_nodes[nodeIndex].is_leaf());
    // An inner node is never the last node: its left subtree follows it.
    GEO_ASSERT(std::size_t{nodeIndex} + 1 < m_nodes.size());
    return static_cast<index_type>(nodeIndex + 1);
  }

  /**
   * \brief Index into nodes() of the right child of the inner node at \p nodeIndex.
   *
   * \pre The node at \p nodeIndex is an inner node.
   * O(1).
   */
  GEO_NODISCARD index_type right_child_idx(const index_type nodeIndex) const noexcept
  {
    GEO_ASSERT(nodeIndex < m_nodes.size());
    GEO_ASSERT(!m_nodes[nodeIndex].is_leaf());
    // Pre-order: the right child comes after the non-empty left subtree and lies inside storage.
    GEO_ASSERT(std::size_t{nodeIndex} + 1 < m_nodes[nodeIndex].right_child_idx());
    GEO_ASSERT(m_nodes[nodeIndex].right_child_idx() < m_nodes.size());
    return m_nodes[nodeIndex].right_child_idx();
  }

private:
  std::vector<Node> m_nodes;
  std::vector<IndexedBox> m_indexedBoxes;

  /**
   * \brief Smallest box containing the indexed boxes [begin, end).
   *
   * \pre begin < end <= indexed box count.
   * O(end - begin).
   */
  GEO_NODISCARD Box enclosing_bounds(const std::size_t begin, const std::size_t end) const noexcept
  {
    GEO_ASSERT(begin < end);
    GEO_ASSERT(end <= m_indexedBoxes.size());

    Box bounds = m_indexedBoxes[begin].box;
    for (std::size_t i = begin + 1; i < end; ++i)
    {
      bounds = detail::enclosing_box(bounds, m_indexedBoxes[i].box);
    }
    return bounds;
  }

  /**
   * \brief Axis along which the centers of the indexed boxes [begin, end) spread most.
   *
   * The center spread, unlike the enclosing bounds, is not stretched by a single long box, so
   * splitting along this axis separates the boxes best.
   *
   * \pre begin < end <= indexed box count.
   * O(end - begin).
   */
  GEO_NODISCARD std::uint8_t widest_center_axis(const std::size_t begin, const std::size_t end) const noexcept
  {
    using Vec3 = linal::vec3<T>;

    GEO_ASSERT(begin < end);
    GEO_ASSERT(end <= m_indexedBoxes.size());

    // Corners of the box enclosing all doubled centers; the doubling scales every axis alike, so the
    // widest axis is unchanged.
    Vec3 centerBoundsMin;
    for (std::uint8_t axis = 0; axis < 3; ++axis)
    {
      centerBoundsMin[axis] = detail::center_key(m_indexedBoxes[begin].box, axis);
    }
    Vec3 centerBoundsMax = centerBoundsMin;
    for (std::size_t i = begin + 1; i < end; ++i)
    {
      for (std::uint8_t axis = 0; axis < 3; ++axis)
      {
        const T key = detail::center_key(m_indexedBoxes[i].box, axis);
        centerBoundsMin[axis] = std::min(centerBoundsMin[axis], key);
        centerBoundsMax[axis] = std::max(centerBoundsMax[axis], key);
      }
    }

    const Vec3 centerSpread{centerBoundsMax - centerBoundsMin};
    std::uint8_t widestAxis = 0;
    for (std::uint8_t axis = 1; axis < 3; ++axis)
    {
      if (centerSpread[axis] > centerSpread[widestAxis])
      {
        widestAxis = axis;
      }
    }
    return widestAxis;
  }

  /**
   * \brief Appends the subtree over the indexed boxes [begin, end) to the node storage in pre-order.
   *
   * Splits at the median of the box centers along \c widest_center_axis; the median keeps both
   * halves equally large whatever the geometry. The split only affects the tree's balance and
   * tightness, never its correctness, because the node bounds are computed exactly.
   */
  void build_subtree(const std::size_t begin, const std::size_t end, const std::size_t depth)
  {
    GEO_ASSERT(begin < end);
    GEO_ASSERT(end <= m_indexedBoxes.size());
    GEO_ASSERT(depth < max_depth);

    const Box bounds = enclosing_bounds(begin, end);
    const std::size_t count = end - begin;
    const Node leaf = Node::create_leaf(bounds, static_cast<index_type>(begin), static_cast<index_type>(count));
    if (count <= max_leaf_size)
    {
      m_nodes.push_back(leaf);
      return;
    }

    // Pre-order needs this node's slot before its children, but the right child's index is only
    // known once the left subtree is built; the leaf holds the slot until then.
    const std::size_t reservedSlot = m_nodes.size();
    m_nodes.push_back(leaf);

    const std::uint8_t splitAxis = widest_center_axis(begin, end);
    const std::size_t middle = begin + count / 2;
    // count > max_leaf_size >= 3 gives each child at least two boxes, which bounds the node count.
    GEO_ASSERT(middle - begin >= 2);
    GEO_ASSERT(end - middle >= 2);
    using IndexedBoxIterator = typename std::vector<IndexedBox>::iterator;
    const IndexedBoxIterator indexedBoxesBegin = m_indexedBoxes.begin();
    std::nth_element(indexedBoxesBegin + static_cast<std::ptrdiff_t>(begin),
                     indexedBoxesBegin + static_cast<std::ptrdiff_t>(middle),
                     indexedBoxesBegin + static_cast<std::ptrdiff_t>(end),
                     [splitAxis](const IndexedBox& lhs, const IndexedBox& rhs) {
                       return detail::center_key(lhs.box, splitAxis) < detail::center_key(rhs.box, splitAxis);
                     });

    // Building the left subtree right after pushing this node places the left child at reservedSlot + 1,
    // which left_child_idx relies on (see Node).
    GEO_ASSERT(m_nodes.size() == reservedSlot + 1);
    build_subtree(begin, middle, depth + 1);
    const std::size_t rightChildIdx = m_nodes.size();
    build_subtree(middle, end, depth + 1);

    // The bounds come from all boxes of the range, independently of the children's, so this
    // cross-checks the invariant that lets the traversals skip a node that misses the query.
    GEO_ASSERT(detail::contains(bounds, m_nodes[reservedSlot + 1].bounds()));
    GEO_ASSERT(detail::contains(bounds, m_nodes[rightChildIdx].bounds()));
    m_nodes[reservedSlot] = Node::create_inner(bounds, static_cast<index_type>(rightChildIdx));
  }
};

/**
 * \brief Tree built by \c AABBTree::create_from_boxes, or the reason it could not be built.
 *
 * A reported failure always carries an empty tree, never a partial one.
 */
template <typename T, typename TIndex>
struct AABBTreeResult
{
  AABBTree<T, TIndex> tree;
  AABBTreeStatus error = AABBTreeStatus::Ok;

  GEO_NODISCARD bool has_value() const noexcept { return error == AABBTreeStatus::Ok; }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

/**
 * \brief Calls \p callback(index) for every box of \p tree that overlaps \p query, each exactly once.
 *
 * Overlap is closed: boxes that only touch \p query are reported. Does not allocate.
 * O(log n + k) for k reported boxes on well-separated input, O(n) in the worst case.
 *
 * \pre \p query has min <= max on every axis.
 */
template <typename T, typename TIndex, typename TCallback>
  requires std::invocable<TCallback&, TIndex>
void for_each_overlapping(const AABBTree<T, TIndex>& tree, const AABB<T, 3>& query, TCallback&& callback) noexcept(
    std::is_nothrow_invocable_v<TCallback&, TIndex>)
{
  using Tree = AABBTree<T, TIndex>;
  using Node = typename Tree::Node;
  using IndexedBox = typename Tree::IndexedBox;

  GEO_ASSERT(detail::has_ordered_bounds(query));
  if (tree.empty())
  {
    return;
  }

  const std::span<const Node> nodes = tree.nodes();
  const std::span<const IndexedBox> indexedBoxes = tree.indexed_boxes();
  // A tree with boxes always has a root at index 0.
  GEO_ASSERT(!nodes.empty());

  // Each step pops one node and pushes at most its two children, so the stack never holds more than
  // depth + 1 nodes.
  detail::FixedStack<TIndex, Tree::max_depth + 1> pending;
  pending.push(TIndex{0});
  while (!pending.empty())
  {
    const TIndex nodeIndex = pending.pop();
    const Node& node = nodes[nodeIndex];
    if (!detail::is_overlapping_closed(node.bounds(), query))
    {
      continue;
    }
    if (node.is_leaf())
    {
      const std::size_t boxEnd = std::size_t{node.first_box_idx()} + node.box_count();
      GEO_ASSERT(boxEnd <= indexedBoxes.size());
      for (std::size_t i = node.first_box_idx(); i < boxEnd; ++i)
      {
        if (detail::is_overlapping_closed(indexedBoxes[i].box, query))
        {
          callback(indexedBoxes[i].inputIndex);
        }
      }
      continue;
    }
    pending.push(tree.right_child_idx(nodeIndex));
    pending.push(tree.left_child_idx(nodeIndex));
  }
}

/**
 * \brief Calls \p callback(firstIndex, secondIndex) for every pair of a box of \p first and a box of
 * \p second that overlap, each ordered pair exactly once.
 *
 * Passing the same tree as both operands is allowed and reports every box paired with itself as
 * well as both orders (i, j) and (j, i) of every overlapping pair.
 *
 * The broad phase of the mesh Booleans: the trees hold the face boxes of the two operands, and every
 * reported pair is a candidate for the exact triangle–triangle test. Overlap is closed, so touching
 * boxes (coplanar contact) are reported. Traverses both trees together, so a subtree pair is
 * discarded as soon as their bounds are disjoint. Does not allocate.
 * O(n + m + k) on well-separated input for k reported pairs, O(n * m) in the worst case.
 */
template <typename T, typename TIndex, typename TCallback>
  requires std::invocable<TCallback&, TIndex, TIndex>
void for_each_overlapping_pair(const AABBTree<T, TIndex>& first,
                               const AABBTree<T, TIndex>& second,
                               TCallback&& callback) noexcept(std::is_nothrow_invocable_v<TCallback&, TIndex, TIndex>)
{
  using Tree = AABBTree<T, TIndex>;
  using Node = typename Tree::Node;
  using IndexedBox = typename Tree::IndexedBox;

  struct NodePair
  {
    TIndex firstNode;
    TIndex secondNode;
  };

  if (first.empty() || second.empty())
  {
    return;
  }

  const std::span<const Node> firstNodes = first.nodes();
  const std::span<const Node> secondNodes = second.nodes();
  const std::span<const IndexedBox> firstBoxes = first.indexed_boxes();
  const std::span<const IndexedBox> secondBoxes = second.indexed_boxes();
  // A tree with boxes always has a root at index 0.
  GEO_ASSERT(!firstNodes.empty());
  GEO_ASSERT(!secondNodes.empty());

  // Each step replaces one node of the popped pair by its two children, adding at most one pending
  // pair per level descended in either tree; the stack never holds more than the two depths + 1 pairs.
  detail::FixedStack<NodePair, 2 * Tree::max_depth + 1> pending;
  pending.push(NodePair{TIndex{0}, TIndex{0}});
  while (!pending.empty())
  {
    const NodePair pair = pending.pop();
    const Node& firstNode = firstNodes[pair.firstNode];
    const Node& secondNode = secondNodes[pair.secondNode];
    if (!detail::is_overlapping_closed(firstNode.bounds(), secondNode.bounds()))
    {
      continue;
    }

    if (firstNode.is_leaf() && secondNode.is_leaf())
    {
      const std::size_t firstEnd = std::size_t{firstNode.first_box_idx()} + firstNode.box_count();
      const std::size_t secondEnd = std::size_t{secondNode.first_box_idx()} + secondNode.box_count();
      GEO_ASSERT(firstEnd <= firstBoxes.size());
      GEO_ASSERT(secondEnd <= secondBoxes.size());
      for (std::size_t i = firstNode.first_box_idx(); i < firstEnd; ++i)
      {
        for (std::size_t j = secondNode.first_box_idx(); j < secondEnd; ++j)
        {
          if (detail::is_overlapping_closed(firstBoxes[i].box, secondBoxes[j].box))
          {
            callback(firstBoxes[i].inputIndex, secondBoxes[j].inputIndex);
          }
        }
      }
      continue;
    }

    const bool descendFirst =
        secondNode.is_leaf() ||
        (!firstNode.is_leaf() && detail::extent_sum(firstNode.bounds()) >= detail::extent_sum(secondNode.bounds()));
    if (descendFirst)
    {
      pending.push(NodePair{first.right_child_idx(pair.firstNode), pair.secondNode});
      pending.push(NodePair{first.left_child_idx(pair.firstNode), pair.secondNode});
    }
    else
    {
      pending.push(NodePair{pair.firstNode, second.right_child_idx(pair.secondNode)});
      pending.push(NodePair{pair.firstNode, second.left_child_idx(pair.secondNode)});
    }
  }
}

} // namespace Geometry

#endif // GEOMETRY_AABBTREE_HPP
