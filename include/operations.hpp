#pragma once

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "attribute.hpp"
#include "base_relation.hpp"
#include "csv.hpp"
#include "json.hpp"
#include "memory_relation.hpp"
#include "relation.hpp"

template<
    typename R,
    typename Un = typename std::enable_if<std::is_base_of<Relation, R>::value>::type>
auto project(R const& relation, std::vector<std::string> const& attributes_names)
    -> MemoryRelation
{
    std::vector<Attribute> new_attributes{};
    std::vector<uint64_t> new_attributes_indexes{};

    auto const& attributes = relation.m_attributes;

    for (std::string const& a_name : attributes_names)
    {
        auto it = std::find_if(attributes.begin(), attributes.end(), [&](Attribute const& a) {
            return a.name == a_name;
        });

        if (it == attributes.end())
        {
            throw std::runtime_error(
                "Attribute " + std::quoted(a_name)._M_string + " not found in relation.");
        }

        new_attributes.emplace_back(*it);
        new_attributes_indexes.push_back(std::distance(attributes.begin(), it));
    }

    // XXX: In truth, the same indexes could still be used.
    MemoryRelation ret{new_attributes, {}, {}};

    for (auto it = relation.begin(); it != relation.end(); ++it)
    {
        nlohmann::json tuple;
        for (auto i : new_attributes_indexes)
        {
            tuple.emplace_back((*it)[i]);
        }

        ret.insert(std::move(tuple));
    }

    return ret;
}

template<
    typename R,
    typename Un = typename std::enable_if<std::is_base_of<Relation, R>::value>::type>
void relation_to_csv(R const& relation, std::ostream& out)
{
    auto writer = csv::make_csv_writer(out);

    std::vector<std::string> v_tuple;
    v_tuple.reserve(relation.m_attributes.size());

    for (Attribute const& a : relation.m_attributes)
    {
        v_tuple.push_back(a.name);
    }
    writer << v_tuple;

    for (nlohmann::json const& tuple : relation)
    {
        v_tuple.clear();
        for (nlohmann::json const& e : tuple)
        {
            if (e.is_string())
            {
                v_tuple.push_back(e);
            }
            else
            {
                v_tuple.push_back(e.dump());
            }
        }

        writer << v_tuple;
    }
}
