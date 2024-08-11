#ifndef GEOMETRY_KDTREE_HPP
#define GEOMETRY_KDTREE_HPP

#include <cmath>
#include <linal/vec.hpp>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Geometry
{

template <typename TKdTreeTraits>
struct NodeValue
{
  using vec_t = typename TKdTreeTraits::vec_t;

  vec_t point{};
};

template <typename TKdTreeTraits>
struct NodePayload
{
  using vec_t = typename TKdTreeTraits::vec_t;
  using payload_t = typename TKdTreeTraits::payload_t;

  vec_t point{};
  payload_t payload{};
};

// clang-format off
struct NoPayloadTag {};
// clang-format on

template <typename TVec, typename TPayload = NoPayloadTag>
struct KdTreeTraits
{
  using value_type = typename TVec::value_type;
  using size_type = typename TVec::size_type;

  using vec_t = TVec;
  using payload_t = TPayload;

  using node_value_t = std::conditional_t<std::is_same_v<payload_t, NoPayloadTag>, NodeValue<KdTreeTraits>, NodePayload<KdTreeTraits>>;

  static constexpr size_type dim = TVec::dim;
};

template <typename TKdTreeTraits>
struct Node
{
  using node_t = Node<TKdTreeTraits>;

  using vec_t = typename TKdTreeTraits::vec_t;
  using payload_t = typename TKdTreeTraits::payload_t;

  using node_value_t = typename TKdTreeTraits::node_value_t;

  node_value_t nodeValue;

  std::unique_ptr<node_t> left;
  std::unique_ptr<node_t> right;

  Node() = default;

  Node(const node_value_t& value)
      : nodeValue(value)
  {
  }

  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;

  Node(Node&&) noexcept = default;
  Node& operator=(Node&&) noexcept = default;
};

template <typename TPointType, typename TPayload = NoPayloadTag>
class KdTree {
public:
  using kdtree_traits_t = KdTreeTraits<TPointType, TPayload>;

  using value_type = typename kdtree_traits_t::value_type;
  using size_type = typename kdtree_traits_t::size_type;

  using vec_t = typename kdtree_traits_t::vec_t;
  using payload_t = typename kdtree_traits_t::payload_t;

  using node_t = Node<kdtree_traits_t>;

  static constexpr size_type D = kdtree_traits_t::dim;

  KdTree()
      : m_root(nullptr)
      , m_size(0)
  {
  }

  KdTree(const KdTree&) = delete;
  KdTree& operator=(const KdTree&) = delete;

  KdTree(KdTree&&) noexcept = default;
  KdTree& operator=(KdTree&&) noexcept = default;

  template <typename = std::enable_if_t<std::is_same_v<payload_t, NoPayloadTag>>>
  bool insert(const vec_t& point)
  {
    node_t newNode;
    newNode.nodeValue.point = point;
    return insertHelper(m_root, newNode, 0);
  }

  template <typename = std::enable_if_t<!std::is_same_v<payload_t, NoPayloadTag>>>
  bool insert(const vec_t& point, const payload_t& payload)
  {
    node_t newNode;
    newNode.nodeValue.point = point;
    newNode.nodeValue.payload = payload;
    return insertHelper(m_root, newNode, 0);
  }

  bool search(const vec_t& point) const { return searchHelper(m_root.get(), point, 0); }

  template <typename = std::enable_if_t<std::is_same_v<payload_t, NoPayloadTag>>>
  vec_t nearest(const vec_t& queryPoint) const
  {
    if (!m_root)
    {
      throw std::runtime_error("KdTree is empty");
    }

    node_t* bestNode = m_root.get();
    value_type bestDist = distance(queryPoint, bestNode->nodeValue.point);
    nearestHelper(m_root.get(), queryPoint, &bestNode, bestDist, 0);
    return bestNode->nodeValue.point;
  }

  template <typename = std::enable_if_t<!std::is_same_v<payload_t, NoPayloadTag>>>
  std::pair<vec_t, payload_t&> nearest(const vec_t& queryPoint) const
  {
    if (!m_root)
    {
      throw std::runtime_error("KdTree is empty");
    }

    node_t* bestNode = m_root.get();
    value_type bestDist = distance(queryPoint, bestNode->nodeValue.point);
    nearestHelper(m_root.get(), queryPoint, &bestNode, bestDist, 0);
    return {bestNode->nodeValue.point, bestNode->nodeValue.payload};
  }

  [[nodiscard]] constexpr size_type size() const { return m_size; }

  [[nodiscard]] constexpr bool empty() const { return m_size == 0; }

private:
  std::unique_ptr<node_t> m_root;
  size_type m_size;

  [[nodiscard]] bool insertHelper(std::unique_ptr<node_t>& node, const node_t& newNode, size_type depth)
  {
    if (!node)
    {
      node = std::make_unique<node_t>(newNode.nodeValue);
      m_size++;
      return true;
    }

    if (node->nodeValue.point == newNode.nodeValue.point)
    {
      return false; // Point already exists
    }

    auto axis = static_cast<size_type>(depth) % D;
    if (newNode.nodeValue.point[axis] < node->nodeValue.point[axis])
    {
      return insertHelper(node->left, newNode, depth + 1);
    }
    else
    {
      return insertHelper(node->right, newNode, depth + 1);
    }
  }

  [[nodiscard]] bool searchHelper(const node_t* node, const vec_t& point, size_type depth) const
  {
    if (!node)
    {
      return false;
    }

    if (node->nodeValue.point == point)
    {
      return true;
    }

    size_type axis = depth % D;
    if (point[axis] < node->nodeValue.point[axis])
    {
      return searchHelper(node->left.get(), point, depth + 1);
    }
    else
    {
      return searchHelper(node->right.get(), point, depth + 1);
    }
  }

  void nearestHelper(node_t* node, const vec_t& query, node_t** bestNode, value_type& bestDist, size_type depth) const
  {
    if (!node)
    {
      return;
    }

    value_type dist = distance(node->nodeValue.point, query);
    if (dist < bestDist)
    {
      *bestNode = node;
      bestDist = dist;
    }

    size_type axis = depth % D;
    value_type diff = query[axis] - node->nodeValue.point[axis];

    nearestHelper((diff < 0) ? node->left.get() : node->right.get(), query, bestNode, bestDist, depth + 1);

    if (std::abs(diff) < bestDist)
    {
      nearestHelper((diff < 0) ? node->right.get() : node->left.get(), query, bestNode, bestDist, depth + 1);
    }
  }

  [[nodiscard]] value_type distance(const vec_t& a, const vec_t& b) const
  {
    value_type sum = 0;
    for (size_type i = 0; i < D; ++i)
    {
      value_type diff = a[i] - b[i];
      sum += diff * diff;
    }
    return std::sqrt(sum);
  }
};

} // namespace Geometry

#endif // GEOMETRY_KDTREE_HPP
