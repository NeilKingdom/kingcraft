#pragma once

#include <list>

template <typename T>
class FIFO
{
public:
    std::list<T> elements;

    FIFO() = delete;
    FIFO(const int capacity) : m_capacity(capacity)
    {}
    ~FIFO() = default;

    void add(T item)
    {
        elements.push_back(item);

        if (elements.size() > m_capacity)
        {
            elements.pop_front();
        }
    }

private:
    int m_capacity;
};
