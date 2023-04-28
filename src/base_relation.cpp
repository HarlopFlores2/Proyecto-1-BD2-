#include <map>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "base_relation.hpp"

Relation::Relation(std::vector<Attribute> attributes, std::map<int, int> indexes)
    : m_attributes{std::move(attributes)},
      m_indexes{std::move(indexes)},
      m_tuple_size{0}
{
    for (Attribute const& a : m_attributes)
    {
        m_tuple_size += a.size();
    }
}
