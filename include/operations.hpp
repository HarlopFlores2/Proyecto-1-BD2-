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
#include "index.hpp"
#include "json.hpp"
#include "memory_relation.hpp"
#include "predicates.hpp"
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
auto select_indexes(R const& relation, std::vector<predicate_type> predicates)
    -> std::vector<uint64_t>
{
    // XXX: Implementing generators could allow `select` to use `select_indexes` without needing
    // the intermediate indexes vector; alas, it's too hard.

    std::optional<std::vector<uint64_t>> vector_o;
    for (index_type& index : relation.m_indexes)
    {
        for (auto p_it = predicates.begin(); p_it != predicates.end(); ++p_it)
        {
            vector_o = operate_index(index, *p_it);
            if (vector_o.has_value())
            {
                predicates.erase(p_it);
                break;
            }
        }
    }

    std::vector<tuple_validator_type> validators;

    for (predicate_type const& p : predicates)
    {
        auto v = std::visit(
            [&](auto&& p) { return p.generate_validator(relation.m_attributes); }, p);
        validators.emplace_back(std::move(v));
    }

    std::vector<uint64_t> ret;

    if (vector_o.has_value())
    {
        for (uint64_t index : vector_o.value())
        {
            nlohmann::json tuple = relation.read(index);

            bool valid_tuple = true;
            for (tuple_validator_type const& tv : validators)
            {
                if (!tv(tuple))
                {
                    valid_tuple = false;
                    break;
                }
            }

            if (valid_tuple)
            {
                ret.push_back(index);
            }
        }

        return ret;
    }
    else
    {
        for (auto it = relation.begin(); it != relation.end(); ++it)
        {
            nlohmann::json const& tuple = *it;

            bool valid_tuple = true;
            for (tuple_validator_type const& tv : validators)
            {
                if (!tv(tuple))
                {
                    valid_tuple = false;
                    break;
                }
            }

            if (valid_tuple)
            {
                ret.push_back(it.calculate_index());
            }
        }

        return ret;
    }
}

template<
    typename R,
    typename Un = typename std::enable_if<std::is_base_of<Relation, R>::value>::type>
auto select(R const& relation, std::vector<predicate_type> const& predicates) -> MemoryRelation
{
    std::vector<tuple_validator_type> validators;

    for (predicate_type const& p : predicates)
    {
        auto v = std::visit(
            [&](auto&& p) { return p.generate_validator(relation.m_attributes); }, p);
        validators.emplace_back(std::move(v));
    }

    MemoryRelation ret{relation.m_attributes, {}, {}};

    for (nlohmann::json const& tuple : relation)
    {
        bool valid_tuple = true;
        for (tuple_validator_type const& tv : validators)
        {
            if (!tv(tuple))
            {
                valid_tuple = false;
                break;
            }
        }

        if (valid_tuple)
        {
            ret.insert(tuple);
        }
    }

    return ret;
}

template<
    typename R,
    typename Un = typename std::enable_if<std::is_base_of<Relation, R>::value>::type>
void remove(R& relation, std::vector<predicate_type> const& predicates)
{
    auto indexes_to_remove = select_indexes(relation, predicates);
    for (auto index : indexes_to_remove)
    {
        relation.remove(index);
    }
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
