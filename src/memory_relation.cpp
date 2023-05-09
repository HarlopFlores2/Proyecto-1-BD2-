#include <iterator>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"
#include "memory_relation.hpp"

MemoryRelation::Iterator::Iterator(MemoryRelation const* mr, uint64_t index)
    : m_mr(mr),
      m_index(index)
{
}

MemoryRelation::Iterator::Iterator(Iterator const& it)
    : Iterator{it.m_mr, it.m_index}
{
}

auto MemoryRelation::Iterator::operator*() const -> reference
{
    return m_mr->m_tuples.at(m_index);
}

auto MemoryRelation::Iterator::operator->() const -> pointer
{
    return std::addressof(**this);
}

auto MemoryRelation::Iterator::operator++() -> Iterator&
{
    ++m_index;

    return *this;
}

auto MemoryRelation::Iterator::operator++(int) -> Iterator
{
    Iterator it{*this};
    ++it;
    return it;
}

auto MemoryRelation::Iterator::operator--() -> Iterator&
{
    --m_index;

    return *this;
}

auto MemoryRelation::Iterator::operator--(int) -> Iterator
{
    Iterator it{*this};
    --it;
    return it;
}

auto operator==(MemoryRelation::Iterator const& a, MemoryRelation::Iterator const& b) -> bool
{
    if (a.m_mr != b.m_mr)
    {
        return false;
    }
    return a.m_index == b.m_index;
}

auto operator!=(MemoryRelation::Iterator const& a, MemoryRelation::Iterator const& b) -> bool
{
    return !(a == b);
}

MemoryRelation::MemoryRelation(
    std::vector<Attribute> attributes,
    std::map<int, int> indexes,
    std::vector<nlohmann::json> tuples)
    : Relation(std::move(attributes), std::move(indexes)),
      m_tuples(std::move(tuples))
{
}

auto MemoryRelation::begin() const -> Iterator
{
    return {this, 0};
}

auto MemoryRelation::end() const -> Iterator
{
    return {this, m_tuples.size()};
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

auto MemoryRelation::read(uint64_t index) const -> nlohmann::json
{
    return m_tuples.at(index);
}
