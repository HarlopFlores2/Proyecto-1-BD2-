#pragma once

#include <map>
#include <vector>

#include "base_relation.hpp"
#include "json.hpp"

struct MemoryRelation : public Relation
{
    std::vector<nlohmann::json> m_tuples;

    MemoryRelation(
        std::vector<Attribute> attributes,
        std::map<int, int> indexes,
        std::vector<nlohmann::json> tuples);

    void insert(nlohmann::json tuple);
};
