#pragma once

#include "common.hpp"
#include "chunk.hpp"

struct ChunkMapKey
{
    int x, y, z;

    ChunkMapKey(const Vec3_t location)
    {
        x = (int)location.x;
        y = (int)location.y;
        z = (int)location.z;
    }
    bool operator==(const ChunkMapKey &chunk_key) const = default;
};

struct ChunkMapHash
{
    size_t operator()(const ChunkMapKey &chunk_key) const
    {
        return (chunk_key.x * 73856093) ^
               (chunk_key.y * 19349663) ^
               (chunk_key.z * 83492791);
    }
};

class ChunkMap
{
public:
    std::unordered_map<ChunkMapKey, std::shared_ptr<Chunk>, ChunkMapHash> map;

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

    void clear()
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

    std::shared_ptr<Chunk> find(const Vec3_t &chunk_location) const
    {
        auto needle = map.find(ChunkMapKey(chunk_location));
        return (needle == map.end()) ? nullptr : needle->second;
    }

    bool contains(const Vec3_t &chunk_location) const
    {
        return map.contains(ChunkMapKey(chunk_location));
    }
};
