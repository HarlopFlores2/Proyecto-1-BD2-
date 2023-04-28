#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "attribute.hpp"

struct Relation
{
    std::vector<Attribute> m_attributes; // Relation schema
    std::map<int, int> m_indexes;

    uint64_t m_tuple_size;

    virtual auto projection(std::vector<std::string> const& attribute_list)
        -> Relation* = delete;
    virtual auto selection(std::vector<int> const& conditions) -> Relation* = delete;

    Relation(std::vector<Attribute> attributes, std::map<int, int> indexes);

    virtual ~Relation() = default;
};
