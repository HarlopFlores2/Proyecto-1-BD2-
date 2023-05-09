#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "attribute.hpp"
#include "index.hpp"

struct Relation
{
public:
    std::vector<Attribute> const m_attributes; // Relation schema
    std::vector<index_type> m_indexes;

    uint64_t const m_tuple_size;

    static auto selection(std::vector<int> const& conditions) -> std::vector<uint64_t>;

    Relation(std::vector<Attribute> attributes, std::vector<index_type> indexes);

    virtual ~Relation() = default;
};
