#include <iterator>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"
#include "memory_relation.hpp"

MemoryRelation::MemoryRelation(
    std::vector<Attribute> attributes,
    std::map<int, int> indexes,
    std::vector<nlohmann::json> tuples)
    : Relation(std::move(attributes), std::move(indexes)),
      m_tuples(std::move(tuples))
{
}

void MemoryRelation::insert(nlohmann::json tuple)
{
    // TODO: Check if primary key exists

    if (!tuple.is_array())
    {
        throw std::runtime_error("'tuple' given wasn't an array");
    }
    if (m_attributes.size() != tuple.size())
    {
        throw std::runtime_error("'tuple' given was the wrong size");
    }

    auto t_it = tuple.begin();
    for (auto a_it = m_attributes.begin(); a_it != m_attributes.end(); a_it++, t_it++)
    {
        if (!a_it->valid_json(*t_it))
        {
            throw std::runtime_error(
                "'tuple' " + std::to_string(std::distance(tuple.begin(), t_it))
                + " element was not of type " + a_it->to_specifier());
        }
    }

    m_tuples.emplace_back(std::move(tuple));
}
