#pragma once

#include "common.hpp"
#include "chunk.hpp"

template<std::size_t N>
class ChunkList : public std::vector<std::shared_ptr<Chunk>>
{
public:
    void push_back(const std::shared_ptr<Chunk> &chunk)
    {
        // Check if already contains chunk
        for (auto i = this->begin(); i != this->end(); ++i)
        {
            if (*chunk.get() == *i->get())
            {
                return;
            }
        }

        // Remove oldest element and shift remaining elements left
        if (this->size() > N)
        {
            this->erase(this->begin());
            for (int i = 0; i < this->size() - 1; ++i)
            {
                this->data()[i] = std::move(this->data()[i + 1]);
            }
            this->pop_back();
        }


        std::cout << "x = " << chunk->location[0] << ", y = " << chunk->location[1] << std::endl;

        std::vector<std::shared_ptr<Chunk>>::push_back(chunk);
    }
};
