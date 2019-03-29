#ifndef QUADTREE_QUADTREE_IMPL_HPP
#define QUADTREE_QUADTREE_IMPL_HPP

#include "quadtree.hpp"
#include <stdexcept>
#include <stack>
#include <vector>
#include <algorithm>

namespace quadtree
{

    //---- Node --------------------------------------------------------------------------------------------------------

    template <typename Data>
    Node<Data>::Node(float x1, float y1, float x2, float y2)
    {
        this->topLeft = Vec2f{x1, y1};
        this->bottomRight = Vec2f{x2, y2};
        this->center = Vec2f{(x1 + x2) / 2, (y1 + y2) / 2};
    }

    template<typename Data>
    std::unique_ptr< Node<Data> >& Node<Data>::getChild(Vec2f pos) {
        if (pos.x < center.x && pos.y < center.y)
        {
            return childTopLeft;
        }
        else if (pos.x < center.x && pos.y >= center.y)
        {
            return childBottomLeft;
        }
        else if (pos.x >= center.x && pos.y < center.y)
        {
            return childTopRight;
        }
        else
        {
            return childBottomRight;
        }
    }

    template<typename Data>
    bool Node<Data>::intersectsWith(Vec2f topLeft, Vec2f bottomRight) const
    {
        if (bottomRight.y < this->topLeft.y) return false;
        if (bottomRight.x < this->topLeft.x) return false;
        if (topLeft.y > this->bottomRight.y) return false;
        if (topLeft.x > this->bottomRight.x) return false;

        return true;
    }

    //---- Tree --------------------------------------------------------------------------------------------------------

    template<typename Data>
    Tree<Data>::Tree(Vec2f topLeft, Vec2f bottomRight) {
        root = std::make_shared< Node<Data> >(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    }

    template <typename Data>
    void Tree<Data>::insert(Vec2f pos, const Data& data)
    {
        // check that it is in the root node
        if (root == nullptr)
        {
            throw std::exception();     // SHOULD never happen, as the root node is initialized in the constructor
        }
        if (pos.x < root->topLeft.x || pos.x > root->bottomRight.x || pos.y < root->topLeft.y || pos.y > root->bottomRight.y)
        {
            throw std::out_of_range("The point is outside of the area that is covered by this quadtree.");
        }

        // find node to insert into
        auto node = root.get();
        auto found = false;
        auto depth = 1;
        do {

            // go to corresponding child node
            auto next = node->getChild(pos).get();
            depth += 1;

            // we are done, when we reached the bottom of the tree
            found = next == nullptr;
            if (!found)
            {
                node = next;
            }

        } while (!found);

        // add child nodes, if necessary
        while (node->points.size() < maxPointsPerNode && depth < maxDepth)
        {
            // create child nodes
            node->childTopLeft = std::make_unique< Node<Data> >(node->topLeft.x, node->topLeft.y, node->center.x, node->center.y);
            node->childBottomRight = std::make_unique< Node<Data> >(node->center.x, node->center.y, node->bottomRight.x, node->bottomRight.y);
            node->childTopRight = std::make_unique< Node<Data> >(node->center.x, node->topLeft.y, node->bottomRight.x, node->center.y);
            node->childBottomLeft = std::make_unique< Node<Data> >(node->topLeft.x, node->center.y, node->center.x, node->bottomRight.y);

            // distribute points between the new child nodes
            for (auto& point: node->points)
            {
                node->getChild(point.position)->points.push_back(point);
            }
            node->points.clear();

            // Continue with the new node that our new point goes in
            node = node->getChild(pos).get();
            depth += 1;
        }

        // insert
        node->points.push_back(Point<Data>{pos, data});

    }

    template <typename Data>
    void Tree<Data>::remove(Vec2f pos, const Data& data)
    {
        // find the child node at the given position
        auto stack = std::stack< Node<Data>* >();
        auto node = root.get();
        auto found = false;
        do 
        {
            auto next = node->getChild(pos).get();
            found = next == nullptr;
            if (!found)
            {
                node = next;
                stack.push(node);
            }
        } 
        while (!found);

        // find & delete child from this node
        auto itChild = std::find_if(node->points.begin(), node->points.end(), 
            [&](const Point<Data>& v) {
                return v.data == data;
            });
        if (itChild != node->points.end())
        {
            auto itLastItem = node->points.end() - 1;
            std::iter_swap(itChild, itLastItem);    // swap to the end
            node->points.erase(itLastItem);
        } 
        else
        {
            throw std::runtime_error("Child was not found at the given position.");
        }

        // delete empty nodes
        while (!stack.empty())
        {
            node = stack.top();
            stack.pop();

            if (node->childTopLeft == nullptr && node->childTopRight == nullptr && node->childBottomLeft == nullptr && node->childBottomRight == nullptr && node->points.empty())
            {
                auto parent = stack.top();
                parent->getChild(pos) = nullptr;
            }
        }

    }


    template<typename Data>
    std::vector< Point<Data> > Tree<Data>::query(Vec2f upperLeft, Vec2f lowerRight) const
    {

        // the stack contains all nodes in the tree that we still need to traverse
        auto stack = std::stack< Node<Data>* >();
        stack.push(root.get());

        auto result = std::vector< Point<Data> >();

        while (!stack.empty())
        {
            // get next node to traverse from stack
            auto node = stack.top();
            stack.pop();

            // queue child nodes for traversal
            if (node->childTopLeft != nullptr && node->childTopLeft->intersectsWith(upperLeft, lowerRight)) 
            {   stack.push(node->childTopLeft.get());   }
            if (node->childTopRight != nullptr && node->childTopRight->intersectsWith(upperLeft, lowerRight)) 
            {   stack.push(node->childTopRight.get());   }
            if (node->childBottomLeft != nullptr && node->childBottomLeft->intersectsWith(upperLeft, lowerRight)) 
            {   stack.push(node->childBottomLeft.get());   }
            if (node->childBottomRight != nullptr && node->childBottomRight->intersectsWith(upperLeft, lowerRight)) 
            {   stack.push(node->childBottomRight.get());   }

            // add points in this node to result
            for (auto& point: node->points) 
            {
                if (point.position.x >= upperLeft.x && point.position.x <= lowerRight.x && point.position.y >= upperLeft.y && point.position.y <= lowerRight.y)
                {
                    result.push_back(point);
                }
            }

        }

        return result;

    }

}

#endif