#pragma once

#include "common.hpp"
#include "chunk.hpp"

struct ChunkMapKey
{
    vec3 key;

    ChunkMapKey() = default;
    ChunkMapKey(const vec3 &chunk_location)
    {
        key[0] = chunk_location[0];
        key[1] = chunk_location[1];
        key[2] = chunk_location[2];
    }

    bool operator==(const ChunkMapKey &chunk_key) const
    {
        return V3_EQ(this->key, chunk_key.key);
    }
};

struct ChunkMapHash
{
    size_t operator()(const ChunkMapKey &chunk_key) const
    {
        size_t hash = 0;
        constexpr size_t prime = 0x9e3779b97f4a7c15ULL;

        for (int i = 0; i < 3; ++i)
        {
            uint32_t bits = std::bit_cast<uint32_t>(chunk_key.key[i]);
            hash ^= std::hash<uint32_t>{}(bits)
                 +  prime
                 + (hash << 6)
                 + (hash >> 2);
        }

        return hash;
    }
};

class ChunkMap
{
public:
    using Map = std::unordered_map<ChunkMapKey, std::shared_ptr<Chunk>, ChunkMapHash>;
    Map map;

    auto begin()
    {
        return map.begin();
    }

    auto end()
    {
        return map.end();
    }

    auto begin() const
    {
        return map.begin();
    }

    auto end() const
    {
        return map.end();
    }

    auto values()
    {
        return map | std::views::values;
    }

    auto values() const
    {
        return map | std::views::values;
    }

    auto clear()
    {
        return map.clear();
    }

    void insert(const std::shared_ptr<Chunk> &chunk)
    {
        map.emplace(ChunkMapKey(chunk->location), chunk);
    }

    template<typename Iter>
    void insert(Iter begin, Iter end)
    {
        for (auto it = begin; it != end; ++it)
        {
            map.insert(*it);
        }
    }

    std::shared_ptr<Chunk> find(const vec3 &chunk_location) const
    {
        auto needle = map.find(ChunkMapKey(chunk_location));
        return needle == map.end() ? nullptr : needle->second;
    }

    bool contains(const vec3 &chunk_location) const
    {
        return map.contains(ChunkMapKey(chunk_location));
    }
};
