#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <Geometry/AABBTree.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>

#include <GeoQik/GeoQik.hpp>

#include <linal/vec.hpp>

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <span>
#include <utility>
#include <vector>

// Builds an AABB tree over the face boxes of a cylinder and a cone, then replays its nodes one per
// step in the order the build creates them, each leaf followed by the face boxes it owns. A second
// row pushes the same shapes into each other, builds one tree per shape, and its last two steps add
// only the face boxes the pair query reports as overlapping, first the cylinder's, then the cone's:
// right arrow / D adds the next drawing, left arrow / A takes it back, space plays and pauses.
namespace {

using Mesh = Geometry::TriangleHalfedgeMesh3d;
using Box = Geometry::AABB3d;
using Tree = Geometry::AABBTree<double>;

constexpr std::size_t boxEdgeCount = 12;
constexpr std::size_t lineValueCount = 6;
constexpr std::size_t colorChannelCount = 4;
// The requested minimum size of the hierarchy; the scene below yields 31 nodes.
constexpr std::size_t minNodeCount = 12;
// Both shapes of make_cylinder_and_cone have 32 faces; the overlap row may draw all boxes of one.
constexpr std::size_t maxFacesPerMesh = 32;

/**
 * \brief Line endpoints and per-line colors of up to \p MaxBoxes boxes on one depth layer, sent to
 * geoqik in a single call.
 *
 * One call makes one replay log entry, so each replay step adds one drawing. Fixed-size buffers,
 * because every drawing is built in a fresh one and its box count is bounded by the scene.
 */
template <std::size_t MaxBoxes>
class BoxLines {
    std::array<double, MaxBoxes * boxEdgeCount * lineValueCount> m_endpoints{};
    std::array<float, MaxBoxes * boxEdgeCount * colorChannelCount> m_colors{};
    std::size_t m_lineCount{0};

    void append_line(const linal::double3& source, const linal::double3& target, const example::color& color) {
        const std::size_t endpointOffset = m_lineCount * lineValueCount;
        std::copy(source.begin(), source.end(), m_endpoints.begin() + static_cast<std::ptrdiff_t>(endpointOffset));
        std::copy(target.begin(), target.end(), m_endpoints.begin() + static_cast<std::ptrdiff_t>(endpointOffset + 3));
        const std::size_t colorOffset = m_lineCount * colorChannelCount;
        std::copy(color.rgba.begin(), color.rgba.end(), m_colors.begin() + static_cast<std::ptrdiff_t>(colorOffset));
        ++m_lineCount;
    }

public:
    /** \brief Appends the 12 edges of \p box. \pre Fewer than MaxBoxes boxes appended so far. */
    void append_box(const Box& box, const example::color& color) {
        GEO_ASSERT(m_lineCount + boxEdgeCount <= MaxBoxes * boxEdgeCount);

        const linal::double3 min = box.get_min();
        const linal::double3 max = box.get_max();
        // Bit i of a corner index selects max over min on axis i.
        const auto corner = [&](const unsigned cornerIndex) {
            linal::double3 position;
            for (unsigned axis = 0; axis < 3; ++axis)
                position[axis] = (cornerIndex & (1u << axis)) != 0 ? max[axis] : min[axis];
            return position;
        };
        // Every edge joins two corners that differ in one axis bit; starting from the corner with
        // that bit cleared visits each edge once.
        for (unsigned cornerIndex = 0; cornerIndex < 8; ++cornerIndex) {
            for (unsigned axis = 0; axis < 3; ++axis) {
                const unsigned axisBit = 1u << axis;
                if ((cornerIndex & axisBit) == 0)
                    append_line(corner(cornerIndex), corner(cornerIndex | axisBit), color);
            }
        }
    }

    /** \brief Adds all appended lines as one geoqik geometry of the given width and depth layer. */
    void draw(const float lineWidth, const std::int32_t depthLayer) const {
        geoqik_add_line_opts_t options{};
        options.color = m_colors.data();
        options.colorCount = m_lineCount * colorChannelCount;
        options.styleSet = 1;
        options.style.lineWidth = lineWidth;
        options.style.depthLayer = depthLayer;
        options.lineType = GEOQIK_LINE_TYPE_LINES;
        const geoqik_result_t result =
            geoqik_add_lines_opts(m_endpoints.data(), m_lineCount * lineValueCount, &options);
        example::check_geoqik(result.err, "Draw tree node");
    }
};

// One color per tree level, so siblings share a color and nesting reads as a color sequence.
example::color level_color(const std::size_t depth) {
    const std::array<example::color, 5> palette{
        example::red(), example::orange(), example::yellow(), example::green(), example::cyan()};
    return palette[depth % palette.size()];
}

// The boxes a leaf owns are tinted lighter than the leaf so the two stay apart where they touch.
example::color owned_box_color(const std::size_t depth) {
    const std::array<example::color, 5> palette{example::light_red(),
                                                example::light_orange(),
                                                example::light_yellow(),
                                                example::light_green(),
                                                example::light_cyan()};
    return palette[depth % palette.size()];
}

// Child bounds often share faces with their parent's. Thinner lines per level keep the parent's
// thicker line visible around the child's, which level_depth_layer puts on top.
float level_line_width(const std::size_t depth) {
    constexpr float rootLineWidth = 6.0f;
    constexpr float widthStepPerLevel = 1.0f;
    constexpr float minLineWidth = 1.5f;
    return std::max(minLineWidth, rootLineWidth - widthStepPerLevel * static_cast<float>(depth));
}

// Lifts the mesh edges off the faces they lie on. The boxes stay above it, so a box edge that
// coincides with a mesh edge (the vertical sides of the cylinder) shows in its level color.
constexpr std::int32_t meshEdgeDepthLayer = 1;

// Where a box's edges coincide with the edges of the larger box enclosing it, plain depth testing
// decides by sub-pixel noise and the two flicker. One layer more per level makes the smaller box
// win.
std::int32_t level_depth_layer(const std::size_t depth) {
    return meshEdgeDepthLayer + 1 + static_cast<std::int32_t>(depth);
}

// Every edge is tagged as a crease because the renderer only outlines creases, and the outline
// should show the triangles whose boxes the tree holds.
template <typename TShape>
Mesh make_outlined_mesh(const TShape& shape, const std::size_t segments, const char* name) {
    auto result = Geometry::make_triangle_mesh(shape, segments);
    if (!result)
        example::fail_example(name, static_cast<int>(result.error));
    Mesh mesh = std::move(result.mesh);
    for (const auto edge : mesh.edges())
        mesh.set_crease(edge, true);
    return mesh;
}

// Both rows show the same two shapes, only placed differently, so the overlap row reads as the
// first one pushed together. 32 faces each: the median split at the root of a tree over both
// separates the two shapes, and the 64 face boxes give a balanced tree of 31 nodes over four levels
// below the root.
std::array<Mesh, 2> make_cylinder_and_cone(const double cylinderX, const double coneX, const double y) {
    constexpr std::size_t cylinderSegments = 8;
    constexpr std::size_t coneSegments = 16;
    constexpr double radius = 1.5;
    constexpr double height = 3.0;
    return {make_outlined_mesh(Geometry::Cylinder<double>{Geometry::Segment3d{{cylinderX, y, 0}, {cylinderX, y, height}},
                                                          radius},
                               cylinderSegments,
                               "Create cylinder"),
            make_outlined_mesh(Geometry::Cone<double>{{coneX, y, 0}, {coneX, y, height}, radius},
                               coneSegments,
                               "Create cone")};
}

// The faces are translucent: geoqik draws translucent meshes without writing depth, so the boxes
// inside the closed meshes stay visible. The edges are black, a color no box uses, so mesh edges and
// box edges stay apart.
void draw_translucent_mesh(const Mesh& mesh, const example::color& color) {
    constexpr float surfaceAlpha = 0.35f;
    constexpr float edgeLineWidth = 1.0f;
    const example::color translucent{color.rgba[0], color.rgba[1], color.rgba[2], surfaceAlpha};
    example::draw(mesh, translucent, edgeLineWidth, meshEdgeDepthLayer);
}

// Set through set_min / set_max because the (min, max) constructor rejects the flat boxes of
// axis-aligned faces, such as the cap triangles.
Box face_box(const Mesh& mesh, const Mesh::FaceHandle face) {
    linal::double3 min{std::numeric_limits<double>::max()};
    linal::double3 max{std::numeric_limits<double>::lowest()};
    for (auto vertex = mesh.vertices(face).circulator(); vertex.is_valid(); ++vertex) {
        const linal::double3 position = mesh.get_position(vertex.get_vertexhandle());
        for (std::uint8_t axis = 0; axis < 3; ++axis) {
            min[axis] = std::min(min[axis], position[axis]);
            max[axis] = std::max(max[axis], position[axis]);
        }
    }
    Box box;
    box.set_min(min);
    box.set_max(max);
    return box;
}

std::vector<Box> face_boxes(const std::span<const Mesh> meshes) {
    std::size_t faceCount = 0;
    for (const Mesh& mesh : meshes)
        faceCount += mesh.face_count();
    std::vector<Box> boxes;
    boxes.reserve(faceCount);
    for (const Mesh& mesh : meshes)
        for (const auto face : mesh.faces())
            boxes.push_back(face_box(mesh, face));
    return boxes;
}

Tree build_tree(const std::span<const Box> boxes) {
    auto treeResult = Tree::create_from_boxes(boxes);
    if (!treeResult)
        example::fail_example("Build tree", static_cast<int>(treeResult.error));
    return std::move(treeResult.tree);
}

// Pre-order storage puts every child after its parent, so one forward pass sets each node's depth
// before the loop reaches it.
std::vector<std::size_t> node_depths(const Tree& tree) {
    using Node = Tree::Node;
    using Index = Tree::index_type;

    const std::span<const Node> nodes = tree.nodes();
    std::vector<std::size_t> depths(nodes.size(), 0);
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].is_leaf())
            continue;
        const auto nodeIndex = static_cast<Index>(i);
        depths[tree.left_child_idx(nodeIndex)] = depths[i] + 1;
        depths[tree.right_child_idx(nodeIndex)] = depths[i] + 1;
    }
    return depths;
}

/**
 * \brief Draws the node at \p nodeIndex and, for a leaf, the face boxes it owns, so the leaf's
 * next step shows which triangles it holds.
 *
 * The owned boxes are a drawing of their own because a drawing has a single depth layer, and they
 * are smaller than the leaf, so they belong one layer above it.
 *
 * \return The number of drawings added, each one replay step.
 */
[[nodiscard]] std::size_t draw_node(const Tree& tree, const std::size_t nodeIndex, const std::size_t depth) {
    using Node = Tree::Node;
    using IndexedBox = Tree::IndexedBox;

    const Node& node = tree.nodes()[nodeIndex];
    BoxLines<1> bounds;
    bounds.append_box(node.bounds(), level_color(depth));
    bounds.draw(level_line_width(depth), level_depth_layer(depth));
    if (!node.is_leaf())
        return 1;

    BoxLines<Tree::max_leaf_size> ownedBoxes;
    const std::span<const IndexedBox> owned = tree.indexed_boxes().subspan(node.first_box_idx(), node.box_count());
    for (const IndexedBox& indexedBox : owned)
        ownedBoxes.append_box(indexedBox.box, owned_box_color(depth));
    ownedBoxes.draw(level_line_width(depth + 1), level_depth_layer(depth + 1));
    return 2;
}

/**
 * \brief Builds one tree per mesh and draws only the face boxes that overlap a face box of the other
 * mesh: the candidates the broad phase of a Boolean hands to the exact triangle test.
 *
 * Each mesh gets its own drawing, and so its own replay step, so it stays clear which operand a box
 * belongs to. A face box usually overlaps several boxes of the other mesh; the flags draw it once.
 * The colors are ones the tree levels do not use, so the overlap boxes do not read as tree nodes.
 *
 * \return The number of drawings added, each one replay step.
 */
[[nodiscard]] std::size_t draw_overlapping_face_boxes(const std::array<Mesh, 2>& meshes) {
    using Index = Tree::index_type;
    using OverlapFlags = std::bitset<maxFacesPerMesh>;

    constexpr float overlapLineWidth = 2.0f;
    constexpr std::int32_t overlapDepthLayer = meshEdgeDepthLayer + 1;

    for (const Mesh& mesh : meshes)
        if (mesh.face_count() > maxFacesPerMesh)
            example::fail_example("Fit the overlap row's faces", static_cast<int>(mesh.face_count()));

    const std::span<const Mesh> meshSpan{meshes};
    const std::vector<Box> cylinderBoxes = face_boxes(meshSpan.subspan(0, 1));
    const std::vector<Box> coneBoxes = face_boxes(meshSpan.subspan(1, 1));
    const Tree cylinderTree = build_tree(cylinderBoxes);
    const Tree coneTree = build_tree(coneBoxes);

    OverlapFlags cylinderOverlaps;
    OverlapFlags coneOverlaps;
    Geometry::for_each_overlapping_pair(cylinderTree, coneTree, [&](const Index cylinderFace, const Index coneFace) {
        cylinderOverlaps.set(cylinderFace);
        coneOverlaps.set(coneFace);
    });
    // Guards the scene placement: shapes moved apart would leave the row without boxes.
    if (cylinderOverlaps.none())
        example::fail_example("Find overlapping face boxes", 0);

    const auto draw_flagged = [&](const std::vector<Box>& boxes, const OverlapFlags& flags, const example::color& color) {
        BoxLines<maxFacesPerMesh> lines;
        for (std::size_t i = 0; i < boxes.size(); ++i)
            if (flags.test(i))
                lines.append_box(boxes[i], color);
        lines.draw(overlapLineWidth, overlapDepthLayer);
    };
    draw_flagged(cylinderBoxes, cylinderOverlaps, example::magenta());
    draw_flagged(coneBoxes, coneOverlaps, example::blue());
    return 2;
}

void start_step_through(const std::size_t stepEntryCount) {
    // Slow enough to follow the build when space starts automatic playback.
    constexpr double replayEntriesPerSecond = 2.0;

    geoqik_replay_options_t options{};
    options.entriesPerSecond = replayEntriesPerSecond;
    options.startPaused = 1;
    example::check_geoqik(geoqik_replay_current_log(&options), "Start replay");

    std::size_t currentEntry = 0;
    std::size_t totalEntries = 0;
    example::check_geoqik(geoqik_get_replay_progress(&currentEntry, &totalEntries), "Query replay progress");
    if (totalEntries < currentEntry + stepEntryCount)
        example::fail_example("Find step drawings in replay log", 0);
    const std::size_t setupEntryCount = totalEntries - currentEntry - stepEntryCount;
    if (setupEntryCount > 0)
        example::check_geoqik(geoqik_step_replay_n(setupEntryCount), "Replay scene setup");
}

} // namespace

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();

    // The centers are 2 apart against radii summing to 3, so the shapes overlap around x = 0; the
    // bottom caps share the plane z = 0, so touching cap boxes show that overlap is closed.
    constexpr double overlapRowY = 5.0;
    const std::array<Mesh, 2> meshes = make_cylinder_and_cone(-3.0, 3.0, 0.0);
    const std::array<Mesh, 2> overlapMeshes = make_cylinder_and_cone(-1.0, 1.0, overlapRowY);
    for (const Mesh& mesh : meshes)
        draw_translucent_mesh(mesh, example::grey());
    for (const Mesh& mesh : overlapMeshes)
        draw_translucent_mesh(mesh, example::grey());

    const std::vector<Box> boxes = face_boxes(meshes);
    const Tree tree = build_tree(boxes);
    if (tree.nodes().size() < minNodeCount)
        example::fail_example("Build a tree of at least 12 nodes", static_cast<int>(tree.nodes().size()));

    example::check_geoqik(geoqik_draw(), "Open visualization");

    // nodes() is in pre-order, which is the order the top-down build creates the nodes: each node
    // takes its slot before its subtrees are built. So drawing in storage order replays the build.
    const std::vector<std::size_t> depths = node_depths(tree);
    std::size_t stepEntryCount = 0;
    for (std::size_t i = 0; i < tree.nodes().size(); ++i)
        stepEntryCount += draw_node(tree, i, depths[i]);
    stepEntryCount += draw_overlapping_face_boxes(overlapMeshes);

    start_step_through(stepEntryCount);

    example::check_geoqik(geoqik_wait_for_exit_and_cleanup(), "Close visualization");
    return EXIT_SUCCESS;
}
