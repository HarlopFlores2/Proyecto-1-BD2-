#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "base_relation.hpp"

Relation::Relation(std::vector<Attribute> attributes, std::map<int, int> indexes)
    : m_attributes{std::move(attributes)},
      m_indexes{std::move(indexes)},
      m_tuple_size{[&]() {
          uint64_t ret = 0;
          for (Attribute const& a : m_attributes)
          {
              ret += a.size();
          }
          return ret;
      }()}
{
}