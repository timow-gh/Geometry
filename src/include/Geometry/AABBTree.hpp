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
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr bool is_overlapping_closed(const AABB<T, 3>& lhs, const AABB<T, 3>& rhs) noexcept
{
  using Vec3 = linal::vec3<T>;

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
 * \brief Smallest box containing both \p lhs and \p rhs.
 *
 * Computed with exact min/max instead of \c AABB::add: \c add ignores growth below its tolerance, so
 * a child could stick out of its parent and the traversal would skip an overlapping pair.
 * O(1).
 */
template <typename T>
GEO_NODISCARD constexpr AABB<T, 3> enclosing_box(const AABB<T, 3>& lhs, const AABB<T, 3>& rhs) noexcept
{
  using Vec3 = linal::vec3<T>;

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

/**
 * \brief Static bounding volume hierarchy over 3D boxes; finds the boxes that overlap a query box,
 * or the overlapping box pairs of two trees (see \c for_each_overlapping and
 * \c for_each_overlapping_pair).
 *
 * Built once and never updated: the mesh Booleans query the face boxes of meshes that do not change
 * during the broad phase. Queries report a box by its position in the span given to the
 * constructor, so callers map results back to faces themselves.
 *
 * Overlap is closed and exact (\c detail::is_overlapping_closed): touching and flat boxes are
 * reported, because the broad phase must never miss a contact.
 *
 * Nodes live in one flat vector in depth-first order, the left child directly after its parent, so
 * building costs two allocations and traversal walks contiguous memory. The tree is built top down
 * by median split, which halves the box count per level and bounds the depth by
 * log2(size) independent of the geometry; traversals rely on that bound for their fixed stacks.
 *
 * \tparam T coordinate type.
 * \tparam TIndex unsigned type of the reported box positions.
 */
template <typename T, typename TIndex = std::uint32_t>
class AABBTree {
  static_assert(std::is_unsigned_v<TIndex>, "box positions are unsigned indices");

public:
  using Box = AABB<T, 3>;
  using index_type = TIndex;

  // Small leaves trade a few extra box tests for fewer nodes. At least 3 so that splitting a node
  // gives each child at least two boxes, which bounds the node count by the box count.
  static constexpr std::size_t max_leaf_size = 4;
  static_assert(max_leaf_size >= 3);

  // Median splits give depth <= ceil(log2(size)) < 64 for any index type up to 64 bits; it sizes the
  // traversal stacks.
  static constexpr std::size_t max_depth = 64;

  /**
   * \brief One box of the input together with its position in the constructor's span.
   *
   * The build reorders items so that every leaf owns a contiguous range of them.
   */
  struct Item
  {
    Box box;
    index_type index{};
  };

  /**
   * \brief A node of the hierarchy; a leaf if \c itemCount is non-zero.
   *
   * A leaf owns items [firstItem, firstItem + itemCount). An inner node's left child is the next
   * node in storage, its right child is \c rightChild.
   */
  struct Node
  {
    Box bounds;
    index_type firstItem{};
    index_type itemCount{};
    index_type rightChild{};

    GEO_NODISCARD constexpr bool is_leaf() const noexcept { return itemCount != 0; }
  };

  AABBTree() noexcept = default;

  /**
   * \brief Builds the hierarchy over \p boxes. O(n log n).
   *
   * \pre Every box has min <= max on every axis (flat boxes are fine).
   */
  explicit AABBTree(const std::span<const Box> boxes)
  {
    GEO_ASSERT(boxes.size() <= std::numeric_limits<index_type>::max());

    m_items.reserve(boxes.size());
    for (std::size_t i = 0; i < boxes.size(); ++i)
    {
      GEO_ASSERT(detail::has_ordered_bounds(boxes[i]));
      m_items.push_back(Item{boxes[i], static_cast<index_type>(i)});
    }
    if (m_items.empty())
    {
      return;
    }

    // Every leaf except a lone root holds at least two boxes (see max_leaf_size), so a binary tree
    // with at most size / 2 leaves has fewer than size nodes.
    m_nodes.reserve(m_items.size());
    build_subtree(0, m_items.size(), 0);
    GEO_ASSERT(m_nodes.size() <= m_items.size());
  }

  /** \brief Number of boxes in the tree. */
  GEO_NODISCARD std::size_t size() const noexcept { return m_items.size(); }

  GEO_NODISCARD bool empty() const noexcept { return m_items.empty(); }

  /** \brief Nodes in depth-first order; the root is the first node. Read-only view for traversals and tests. */
  GEO_NODISCARD std::span<const Node> nodes() const noexcept { return m_nodes; }

  /** \brief Items in leaf order. Read-only view for traversals and tests. */
  GEO_NODISCARD std::span<const Item> items() const noexcept { return m_items; }

private:
  std::vector<Node> m_nodes;
  std::vector<Item> m_items;

  /**
   * \brief Appends the subtree over items [begin, end) to the node storage in depth-first order.
   *
   * Splits at the median along the axis on which the box centers spread most. The center spread,
   * unlike the node's bounds, is not stretched by a single long box, so it separates the boxes
   * best; the median keeps both halves equally large whatever the geometry.
   */
  void build_subtree(const std::size_t begin, const std::size_t end, const std::size_t depth)
  {
    using Vec3 = linal::vec3<T>;

    GEO_ASSERT(begin < end);
    GEO_ASSERT(depth < max_depth);

    Box bounds = m_items[begin].box;
    // Twice the centers: avoids a division and keeps the comparisons exact.
    Vec3 centerMin{m_items[begin].box.get_min() + m_items[begin].box.get_max()};
    Vec3 centerMax{centerMin};
    for (std::size_t i = begin + 1; i < end; ++i)
    {
      const Box& box = m_items[i].box;
      bounds = detail::enclosing_box(bounds, box);
      const Vec3 center{box.get_min() + box.get_max()};
      for (std::uint8_t axis = 0; axis < 3; ++axis)
      {
        centerMin[axis] = std::min(centerMin[axis], center[axis]);
        centerMax[axis] = std::max(centerMax[axis], center[axis]);
      }
    }

    const std::size_t nodeIndex = m_nodes.size();
    const std::size_t count = end - begin;
    m_nodes.push_back(Node{bounds, static_cast<index_type>(begin), static_cast<index_type>(count), index_type{0}});
    if (count <= max_leaf_size)
    {
      return;
    }

    const Vec3 centerSpread{centerMax - centerMin};
    std::uint8_t splitAxis = 0;
    for (std::uint8_t axis = 1; axis < 3; ++axis)
    {
      if (centerSpread[axis] > centerSpread[splitAxis])
      {
        splitAxis = axis;
      }
    }

    const std::size_t middle = begin + count / 2;
    using ItemIterator = typename std::vector<Item>::iterator;
    const ItemIterator itemsBegin = m_items.begin();
    std::nth_element(itemsBegin + static_cast<std::ptrdiff_t>(begin),
                     itemsBegin + static_cast<std::ptrdiff_t>(middle),
                     itemsBegin + static_cast<std::ptrdiff_t>(end),
                     [splitAxis](const Item& lhs, const Item& rhs) {
                       return lhs.box.get_min()[splitAxis] + lhs.box.get_max()[splitAxis] <
                              rhs.box.get_min()[splitAxis] + rhs.box.get_max()[splitAxis];
                     });

    build_subtree(begin, middle, depth + 1);
    const std::size_t rightChild = m_nodes.size();
    build_subtree(middle, end, depth + 1);

    // Indexed access: a reference taken before the recursion would not survive a reallocation.
    m_nodes[nodeIndex].itemCount = index_type{0};
    m_nodes[nodeIndex].rightChild = static_cast<index_type>(rightChild);
  }
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
  using Item = typename Tree::Item;

  GEO_ASSERT(detail::has_ordered_bounds(query));
  if (tree.empty())
  {
    return;
  }

  const std::span<const Node> nodes = tree.nodes();
  const std::span<const Item> items = tree.items();

  // Each step pops one node and pushes at most its two children, so the stack never holds more than
  // depth + 1 nodes.
  detail::FixedStack<TIndex, Tree::max_depth + 1> pending;
  pending.push(TIndex{0});
  while (!pending.empty())
  {
    const TIndex nodeIndex = pending.pop();
    const Node& node = nodes[nodeIndex];
    if (!detail::is_overlapping_closed(node.bounds, query))
    {
      continue;
    }
    if (node.is_leaf())
    {
      for (std::size_t i = node.firstItem; i < std::size_t{node.firstItem} + node.itemCount; ++i)
      {
        if (detail::is_overlapping_closed(items[i].box, query))
        {
          callback(items[i].index);
        }
      }
      continue;
    }
    pending.push(node.rightChild);
    pending.push(static_cast<TIndex>(nodeIndex + 1));
  }
}

/**
 * \brief Calls \p callback(firstIndex, secondIndex) for every pair of a box of \p first and a box of
 * \p second that overlap, each pair exactly once.
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
  using Item = typename Tree::Item;

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
  const std::span<const Item> firstItems = first.items();
  const std::span<const Item> secondItems = second.items();

  // Each step replaces one node of the popped pair by its two children, adding at most one pending
  // pair per level descended in either tree; the stack never holds more than the two depths + 1 pairs.
  detail::FixedStack<NodePair, 2 * Tree::max_depth + 1> pending;
  pending.push(NodePair{TIndex{0}, TIndex{0}});
  while (!pending.empty())
  {
    const NodePair pair = pending.pop();
    const Node& firstNode = firstNodes[pair.firstNode];
    const Node& secondNode = secondNodes[pair.secondNode];
    if (!detail::is_overlapping_closed(firstNode.bounds, secondNode.bounds))
    {
      continue;
    }

    if (firstNode.is_leaf() && secondNode.is_leaf())
    {
      const std::size_t firstEnd = std::size_t{firstNode.firstItem} + firstNode.itemCount;
      const std::size_t secondEnd = std::size_t{secondNode.firstItem} + secondNode.itemCount;
      for (std::size_t i = firstNode.firstItem; i < firstEnd; ++i)
      {
        for (std::size_t j = secondNode.firstItem; j < secondEnd; ++j)
        {
          if (detail::is_overlapping_closed(firstItems[i].box, secondItems[j].box))
          {
            callback(firstItems[i].index, secondItems[j].index);
          }
        }
      }
      continue;
    }

    const bool descendFirst =
        secondNode.is_leaf() ||
        (!firstNode.is_leaf() && detail::extent_sum(firstNode.bounds) >= detail::extent_sum(secondNode.bounds));
    if (descendFirst)
    {
      pending.push(NodePair{firstNode.rightChild, pair.secondNode});
      pending.push(NodePair{static_cast<TIndex>(pair.firstNode + 1), pair.secondNode});
    }
    else
    {
      pending.push(NodePair{pair.firstNode, secondNode.rightChild});
      pending.push(NodePair{pair.firstNode, static_cast<TIndex>(pair.secondNode + 1)});
    }
  }
}

} // namespace Geometry

#endif // GEOMETRY_AABBTREE_HPP
