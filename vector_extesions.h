#pragma once
#include <vector>

template<class T>
void add_if_not_presented(std::vector<T*>& vector, T* to_add)
{
    for (auto ltl : vector)
    {
        if (*to_add == *ltl)
            return;
    }

    vector.push_back(to_add);
}

template<class T>
int find_if_presented(const std::vector<T>& vector, const T to_found)
{
    for (int i = 0; i < vector.size(); i++)
    {
        if (*to_found == *(vector[i]))
            return i;
    }

    return -1;
}
