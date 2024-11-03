#include "my_fifo.hpp"

FIFO::FIFO(const int capacity) :
    m_capacity(capacity)
{}

void FIFO::add(Chunk &item)
{
    elements.push_back(item);

    if (elements.size() > m_capacity)
    {
        elements.pop_front();
    }
}
