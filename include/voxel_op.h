#pragma once

#include <cmath>
#include <iostream>
#include <memory>
#include <ostream>
#include <pcl/point_types.h>
#include <vector>

namespace radar
{
namespace voxel_op
{

template <typename NodeT, typename PointT>
bool push_point(NodeT &node, const PointT &point, const std::function<void(const NodeT &, bool)> callback = nullptr)
{
    if (is_contain<NodeT, PointT>(node, point))
    {
        if (node.size > 0)
        {
            float fsize = std::pow(2, node.size - 1);
            bool x_flag = point.x >= node.location.x + fsize * node.resolution.x;
            bool y_flag = point.y >= node.location.y + fsize * node.resolution.y;
            bool z_flag = point.z >= node.location.z + fsize * node.resolution.z;
            int index = (x_flag << 2) + (y_flag << 1) + z_flag;
            if (!node.children[index])
            {
                node.children[index] = std::shared_ptr<NodeT>(
                    new NodeT(node.size - 1, node.resolution,
                              pcl::PointXYZ(node.location.x + (x_flag ? fsize * node.resolution.x : 0),
                                            node.location.y + (y_flag ? fsize * node.resolution.y : 0),
                                            node.location.z + (z_flag ? fsize * node.resolution.z : 0))));
                if (callback != nullptr)
                    callback(*node.children[index], true);
            }
            if (callback != nullptr)
                callback(node, false);
            node._after_push_point(point);
            return push_point<NodeT, PointT>(*node.children[index], point, callback);
        }
        else
        {
            if (callback != nullptr)
                callback(node, false);
            node._after_push_point(point);
            return true;
        }
    }
    else
        return false;
}

template <typename NodeT, typename PointT>
bool is_contain(const NodeT &node, const PointT &point)
{
    float fsize = std::pow(2, node.size);
    float x = node.location.x;
    float y = node.location.y;
    float z = node.location.z;
    return point.x >= x && point.x < x + fsize * node.resolution.x &&
           point.y >= y && point.y < y + fsize * node.resolution.y &&
           point.z >= z && point.z < z + fsize * node.resolution.z;
}

template <typename NodeT> void recursive_traverse(NodeT &node, std::function<void(NodeT &)> func)
{
    func(node);
    if (node.size > 0)
        for (auto &child : node.children)
            if (child)
                recursive_traverse<NodeT>(*child, func);
}

// with index
template <typename NodeT>
void recursive_traverse_idx(NodeT &node, std::function<void(NodeT &, const std::vector<size_t> &)> func,
                        std::vector<size_t> index = std::vector<size_t>())
{
    func(node, index);
    if (node.size > 0)
    {
        index.push_back(0);
        size_t it = index.size() - 1;
        for (; index[it] < 8; index[it]++)
        {
            if (node.children[index[it]])
                recursive_traverse_idx<NodeT>(*(node.children[index[it]]), func, index);
        }
    }
}
}
}
