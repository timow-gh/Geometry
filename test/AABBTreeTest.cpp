#include <Geometry/AABBTree.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

using Vec3 = linal::double3;
using Box = AABB<double, 3>;
using Tree = AABBTree<double>;
using IndexPair = std::pair<std::uint32_t, std::uint32_t>;

Box make_box(const Vec3& min, const Vec3& max)
{
  return detail::make_closed_box(min, max);
}

/**
 * Boxes with integer corners on a small grid, so touching, flat and identical boxes are frequent:
 * exactly the configurations an open or tolerance-based overlap test gets wrong.
 */
std::vector<Box> random_grid_boxes(const std::size_t count, const std::uint32_t seed)
{
  std::mt19937 generator{seed};
  std::uniform_int_distribution<int> cornerDistribution{0, 12};
  std::uniform_int_distribution<int> extentDistribution{0, 3};

  std::vector<Box> boxes;
  boxes.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
  {
    Vec3 min;
    Vec3 max;
    for (std::uint8_t axis = 0; axis < 3; ++axis)
    {
      min[axis] = static_cast<double>(cornerDistribution(generator));
      max[axis] = min[axis] + static_cast<double>(extentDistribution(generator));
    }
    boxes.push_back(make_box(min, max));
  }
  return boxes;
}

std::vector<Box> random_continuous_boxes(const std::size_t count, const std::uint32_t seed)
{
  std::mt19937 generator{seed};
  std::uniform_real_distribution<double> cornerDistribution{-50.0, 50.0};
  std::uniform_real_distribution<double> extentDistribution{0.0, 6.0};

  std::vector<Box> boxes;
  boxes.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
  {
    Vec3 min;
    Vec3 max;
    for (std::uint8_t axis = 0; axis < 3; ++axis)
    {
      min[axis] = cornerDistribution(generator);
      max[axis] = min[axis] + extentDistribution(generator);
    }
    boxes.push_back(make_box(min, max));
  }
  return boxes;
}

std::vector<IndexPair> tree_pairs(const Tree& first, const Tree& second)
{
  std::vector<IndexPair> pairs;
  for_each_overlapping_pair(first, second, [&pairs](const std::uint32_t firstIndex, const std::uint32_t secondIndex) {
    pairs.emplace_back(firstIndex, secondIndex);
  });
  std::sort(pairs.begin(), pairs.end());
  return pairs;
}

std::vector<IndexPair> brute_force_pairs(const std::vector<Box>& first, const std::vector<Box>& second)
{
  std::vector<IndexPair> pairs;
  for (std::uint32_t i = 0; i < first.size(); ++i)
  {
    for (std::uint32_t j = 0; j < second.size(); ++j)
    {
      if (detail::is_overlapping_closed(first[i], second[j]))
      {
        pairs.emplace_back(i, j);
      }
    }
  }
  return pairs;
}

std::vector<std::uint32_t> tree_query(const Tree& tree, const Box& query)
{
  std::vector<std::uint32_t> indices;
  for_each_overlapping(tree, query, [&indices](const std::uint32_t index) { indices.push_back(index); });
  std::sort(indices.begin(), indices.end());
  return indices;
}

std::vector<std::uint32_t> brute_force_query(const std::vector<Box>& boxes, const Box& query)
{
  std::vector<std::uint32_t> indices;
  for (std::uint32_t i = 0; i < boxes.size(); ++i)
  {
    if (detail::is_overlapping_closed(boxes[i], query))
    {
      indices.push_back(i);
    }
  }
  return indices;
}

bool contains(const Box& outer, const Box& inner)
{
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    if (inner.get_min()[axis] < outer.get_min()[axis] || outer.get_max()[axis] < inner.get_max()[axis])
    {
      return false;
    }
  }
  return true;
}

/** Checks the structure the traversals rely on, for the subtree rooted at \p nodeIndex; returns its item count. */
std::size_t expect_valid_subtree(const Tree& tree, const std::size_t nodeIndex)
{
  using Node = Tree::Node;

  const Node& node = tree.nodes()[nodeIndex];
  if (node.is_leaf())
  {
    EXPECT_LE(node.itemCount, Tree::max_leaf_size);
    for (std::size_t i = node.firstItem; i < std::size_t{node.firstItem} + node.itemCount; ++i)
    {
      EXPECT_TRUE(contains(node.bounds, tree.items()[i].box));
    }
    return node.itemCount;
  }

  const std::size_t leftChild = nodeIndex + 1;
  const std::size_t rightChild = node.rightChild;
  EXPECT_LT(nodeIndex, rightChild);
  EXPECT_LT(rightChild, tree.nodes().size());
  EXPECT_TRUE(contains(node.bounds, tree.nodes()[leftChild].bounds));
  EXPECT_TRUE(contains(node.bounds, tree.nodes()[rightChild].bounds));
  return expect_valid_subtree(tree, leftChild) + expect_valid_subtree(tree, rightChild);
}

TEST(AABBTreeTest, is_overlapping_closed_counts_touching_and_flat_boxes)
{
  const Box unitBox = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});

  // Sharing a face, an edge, a corner.
  EXPECT_TRUE(detail::is_overlapping_closed(unitBox, make_box(Vec3{1.0, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0})));
  EXPECT_TRUE(detail::is_overlapping_closed(unitBox, make_box(Vec3{1.0, 1.0, 0.0}, Vec3{2.0, 2.0, 1.0})));
  EXPECT_TRUE(detail::is_overlapping_closed(unitBox, make_box(Vec3{1.0, 1.0, 1.0}, Vec3{2.0, 2.0, 2.0})));

  // Flat boxes, as produced by axis-aligned faces, lying in the unit box's face and in each other.
  const Box flatTop = make_box(Vec3{0.0, 0.0, 1.0}, Vec3{1.0, 1.0, 1.0});
  const Box flatTopShifted = make_box(Vec3{0.5, 0.5, 1.0}, Vec3{1.5, 1.5, 1.0});
  EXPECT_TRUE(detail::is_overlapping_closed(unitBox, flatTop));
  EXPECT_TRUE(detail::is_overlapping_closed(flatTop, flatTopShifted));

  // Point boxes.
  const Box corner = make_box(Vec3{1.0, 1.0, 1.0}, Vec3{1.0, 1.0, 1.0});
  EXPECT_TRUE(detail::is_overlapping_closed(unitBox, corner));
  EXPECT_TRUE(detail::is_overlapping_closed(corner, corner));

  // Separated along one axis only, by a gap far below linal's tolerance.
  const double gap = 1e-12;
  EXPECT_FALSE(detail::is_overlapping_closed(unitBox, make_box(Vec3{1.0 + gap, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0})));
  EXPECT_FALSE(detail::is_overlapping_closed(flatTop, make_box(Vec3{0.0, 0.0, 1.0 + gap}, Vec3{1.0, 1.0, 1.0 + gap})));
}

TEST(AABBTreeTest, enclosing_box_is_exact)
{
  // Growth far below linal's tolerance must not be dropped, or a parent would not contain its child.
  const Box base = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Box grown = make_box(Vec3{-1e-12, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0 + 1e-12});
  const Box enclosing = detail::enclosing_box(base, grown);
  EXPECT_TRUE(contains(enclosing, base));
  EXPECT_TRUE(contains(enclosing, grown));
  EXPECT_EQ(enclosing.get_min()[0], -1e-12);
  EXPECT_EQ(enclosing.get_max()[2], 1.0 + 1e-12);
}

TEST(AABBTreeTest, empty_tree_reports_nothing)
{
  const Tree emptyTree;
  const std::vector<Box> boxes = random_grid_boxes(10, 1);
  const Tree tree{boxes};

  EXPECT_TRUE(emptyTree.empty());
  EXPECT_EQ(emptyTree.size(), 0U);
  EXPECT_TRUE(emptyTree.nodes().empty());
  EXPECT_TRUE(tree_query(emptyTree, make_box(Vec3{-100.0, -100.0, -100.0}, Vec3{100.0, 100.0, 100.0})).empty());
  EXPECT_TRUE(tree_pairs(emptyTree, tree).empty());
  EXPECT_TRUE(tree_pairs(tree, emptyTree).empty());
  EXPECT_TRUE(tree_pairs(emptyTree, emptyTree).empty());

  const std::vector<Box> noBoxes;
  EXPECT_TRUE(Tree{noBoxes}.empty());
}

TEST(AABBTreeTest, single_box_tree)
{
  const std::vector<Box> boxes{make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 0.0})};
  const Tree tree{boxes};

  ASSERT_EQ(tree.size(), 1U);
  ASSERT_EQ(tree.nodes().size(), 1U);
  EXPECT_TRUE(tree.nodes()[0].is_leaf());

  EXPECT_EQ(tree_query(tree, make_box(Vec3{1.0, 1.0, 0.0}, Vec3{2.0, 2.0, 1.0})), std::vector<std::uint32_t>{0});
  EXPECT_TRUE(tree_query(tree, make_box(Vec3{0.0, 0.0, 0.5}, Vec3{1.0, 1.0, 1.0})).empty());
  EXPECT_EQ(tree_pairs(tree, tree), (std::vector<IndexPair>{{0, 0}}));
}

TEST(AABBTreeTest, structure_is_valid_for_all_leaf_boundaries)
{
  // Sizes around multiples of the leaf size exercise every split shape.
  for (std::size_t count = 1; count <= 40; ++count)
  {
    const std::vector<Box> boxes = random_continuous_boxes(count, static_cast<std::uint32_t>(count));
    const Tree tree{boxes};
    ASSERT_EQ(tree.size(), count);
    ASSERT_FALSE(tree.nodes().empty());
    EXPECT_LE(tree.nodes().size(), count);
    EXPECT_EQ(expect_valid_subtree(tree, 0), count);

    // Every input box appears exactly once.
    std::vector<std::uint32_t> indices;
    for (const Tree::Item& item : tree.items())
    {
      indices.push_back(item.index);
      EXPECT_EQ(item.box, boxes[item.index]);
    }
    std::sort(indices.begin(), indices.end());
    for (std::uint32_t i = 0; i < count; ++i)
    {
      EXPECT_EQ(indices[i], i);
    }
  }
}

TEST(AABBTreeTest, pairs_match_brute_force_on_grid_boxes)
{
  for (const std::uint32_t seed : {1U, 2U, 3U})
  {
    const std::vector<Box> firstBoxes = random_grid_boxes(300, seed);
    const std::vector<Box> secondBoxes = random_grid_boxes(170, seed + 100);
    const Tree firstTree{firstBoxes};
    const Tree secondTree{secondBoxes};

    const std::vector<IndexPair> expected = brute_force_pairs(firstBoxes, secondBoxes);
    ASSERT_FALSE(expected.empty());
    // Sorted equality also proves that no pair is reported twice.
    EXPECT_EQ(tree_pairs(firstTree, secondTree), expected);
  }
}

TEST(AABBTreeTest, pairs_match_brute_force_on_continuous_boxes)
{
  const std::vector<Box> firstBoxes = random_continuous_boxes(500, 7);
  const std::vector<Box> secondBoxes = random_continuous_boxes(3, 8);
  const Tree firstTree{firstBoxes};
  const Tree secondTree{secondBoxes};

  EXPECT_EQ(tree_pairs(firstTree, secondTree), brute_force_pairs(firstBoxes, secondBoxes));

  // Swapping the trees swaps every pair.
  std::vector<IndexPair> swapped = tree_pairs(secondTree, firstTree);
  for (IndexPair& pair : swapped)
  {
    std::swap(pair.first, pair.second);
  }
  std::sort(swapped.begin(), swapped.end());
  EXPECT_EQ(swapped, brute_force_pairs(firstBoxes, secondBoxes));
}

TEST(AABBTreeTest, query_matches_brute_force)
{
  const std::vector<Box> boxes = random_grid_boxes(400, 11);
  const Tree tree{boxes};

  for (const Box& query : random_grid_boxes(60, 12))
  {
    EXPECT_EQ(tree_query(tree, query), brute_force_query(boxes, query));
  }
  // A query covering everything reports every box.
  EXPECT_EQ(tree_query(tree, make_box(Vec3{0.0, 0.0, 0.0}, Vec3{15.0, 15.0, 15.0})).size(), boxes.size());
}

TEST(AABBTreeTest, identical_boxes_build_a_balanced_tree)
{
  // All centers coincide, so no axis separates them; the median split still halves the count.
  const std::vector<Box> boxes(300, make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 1.0}));
  const Tree tree{boxes};

  EXPECT_EQ(expect_valid_subtree(tree, 0), boxes.size());
  EXPECT_EQ(tree_query(tree, make_box(Vec3{1.0, 0.0, 1.0}, Vec3{2.0, 1.0, 2.0})).size(), boxes.size());
  EXPECT_EQ(tree_pairs(tree, tree).size(), boxes.size() * boxes.size());
}

} // namespace
