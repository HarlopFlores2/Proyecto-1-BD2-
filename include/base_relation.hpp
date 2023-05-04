#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "attribute.hpp"

struct Relation
{
public:
    std::vector<Attribute> const m_attributes; // Relation schema
    std::map<int, int> m_indexes;

    uint64_t const m_tuple_size;

    static auto selection(std::vector<int> const& conditions) -> std::vector<uint64_t>;

    Relation(std::vector<Attribute> attributes, std::map<int, int> indexes);

    virtual ~Relation() = default;
};
