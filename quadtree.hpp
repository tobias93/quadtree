#ifndef QUADTREE_QUADTREE_HPP
#define QUADTREE_QUADTREE_HPP

#include <memory>
#include <vector>

namespace quadtree {

    /**
     * An x/y coordinate.
     */
    struct Vec2f
    {
        float x;
        float y;
    };

    /**
     * A leaf node of the quadtree, consisting out of a position and some "payload data"
     * that is linked to this position.
     */
    template <typename Data>
    struct Point
    {
        Vec2f position;
        Data data;
    };

    /**
     * A node of the quadtree.
     *
     * A node contains all points in a certain rectangular area of the complete 2d space. Depending on
     * the position of the point relative to the center point of this area, it goes into one out of four different
     * child-nodes.
     */
    template <typename Data>
    struct Node
    {
        Vec2f topLeft;
        Vec2f bottomRight;
        Vec2f center;

        std::unique_ptr<Node> childTopLeft;
        std::unique_ptr<Node> childTopRight;
        std::unique_ptr<Node> childBottomLeft;
        std::unique_ptr<Node> childBottomRight;

        std::vector< Point<Data> > points;

        /**
         * Returns a reference to the child, that the given coordinate belongs to.
         */
        std::unique_ptr<Node>& getChild(Vec2f pos);

        /**
         * Cheecks, if the node intersects with the rectangle defined by the parameters `topLeft` and `bottomRight`.
         */
        bool intersectsWith(Vec2f topLeft, Vec2f bottomRight) const;

        /**
         * Constructs an empty leaf node with a given positioning
         */
        Node(float x1, float y1, float x2, float y2);
    };

    /**
     * A Quadtree.
     *
     * Stores points, consisting out of a coordinate in 2d space and some "Payload" of type `Data`.
     * The points in the quadtree can be queried based on their position.
     */
    template <typename Data>
    class Tree {
    private:

        // the root node of the tree
        std::shared_ptr< Node<Data> > root;

        // control the depth of the quadtree
        const int maxPointsPerNode = 5;
        const int maxDepth = 5;

    public:

        /**
         * Default constructor.
         *
         * Makes a quadtree that can contain coordinates with 0.0 <= x/y <= 1.0
         */
        Tree(): Tree(Vec2f{0, 0}, Vec2f{1, 1}) {};

        /**
         * Constructs a quadtree, that can contain points in the rectangle defined by the given upper left and lower right corner.
         */
        Tree(Vec2f topLeft, Vec2f bottomRight);

        /**
         * Inserts a point into the quadtree
         */
        void insert(Vec2f pos, const Data& data);

        /**
         * Removes the given element from the tree
         */
        void remove(Vec2f pos, const Data& data);

        /**
         * Queries the quadtree.
         *
         * The query filters the points such that only points in the rectangle defined by `upperLeft` and `lowerRight`
         * will be included in the result.
         *
         * @return
         */
        std::vector< Point<Data> > query(Vec2f upperLeft, Vec2f lowerRight) const;

    };

}

#endif

#include "quadtree_impl.hpp"
