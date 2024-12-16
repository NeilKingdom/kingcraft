#pragma once

#include "common.hpp"
#include "chunk.hpp"

// Order by nearest to camera
struct ChunkSort
{
    bool operator()(const std::shared_ptr<Chunk> &a, const std::shared_ptr<Chunk> &b) const
    {
        return (a->location[0] < b->location[0]) ? true
            : (a->location[1] < b->location[1]) ? true : false;
    }
};

template<std::size_t cap>
class ChunkList : public std::set<std::shared_ptr<Chunk>, ChunkSort>
{
public:
    std::pair<iterator, bool> insert(const std::shared_ptr<Chunk> &chunk) {
        if (this->size() >= cap) {
            this->erase(insert_order.front());
            insert_order.erase(insert_order.begin());

            std::rotate(insert_order.begin(), insert_order.begin() + 1, insert_order.end());
        }

        insert_order.push_back(chunk);
        return std::set<std::shared_ptr<Chunk>, ChunkSort>::insert(chunk);
    }

private:
    std::vector<std::shared_ptr<Chunk>> insert_order; // Track oldest to newest insertions
};
