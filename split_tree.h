#pragma once

#include <memory>

template<class DATA>
class Node
{
public:
    DATA data;
    std::shared_ptr<Node> first = nullptr;
    std::shared_ptr<Node> second = nullptr;

    Node(DATA data) : data(data) { }

    std::shared_ptr<Node> set_first(std::shared_ptr<Node> ref) { first = ref; return first; }
    std::shared_ptr<Node> set_first(DATA value) { first = new Node(value); return first; }

    std::shared_ptr<Node> set_second(std::shared_ptr<Node> ref) { second = ref; return first; }
    std::shared_ptr<Node> set_second(DATA value) { second = new Node(value); return first; }

    int nodes_count()
    {
        int res = 1;
        if (first.get())
            res += first->nodes_count();
        if (second.get())
            res += second->nodes_count();

        return res;
    }

    int leafs_count()
    {
        int res = (!first.get() && !second.get()) ? 1 : 0;
        if (first.get())
            res += first->leafs_count();
        if (second.get())
            res += second->leafs_count();

        return res;
    }

    int depth()
    {
        int first_depth = first.get() ? first->depth() + 1 : 0;
        int second_depth = second.get() ? second->depth() + 1 : 0;

        int max_depth = first_depth > second_depth ? first_depth : second_depth;

        return max_depth > 1 ? max_depth : 1;
    }
};
