#pragma once

#include <deque>
#include "chunk.hpp"

class FIFO
{
public:
    std::deque<Chunk> elements;

    FIFO() = delete;
    FIFO(const int capacity);
    ~FIFO() = default;

    void add(Chunk &item);

private:
    int m_capacity;
};
