#include <cassert>
#include <cstdint>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"
#include "parser.hpp"
#include "predicates.hpp"

#include "tao/pegtl.hpp"
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>

#include "prettyprint.hpp"
#include "tao/pegtl/string_input.hpp"

using json = nlohmann::json;

namespace pegtl = tao::pegtl;

using p_node = pegtl::parse_tree::node;

auto literal_node_to_value(p_node const& node) -> json
{
    if (node.is_type<lit_number>())
    {
        return json(std::stoull(node.string()));
    }
    else if (node.is_type<lit_string>())
    {
        return json(node.string());
    }
    else
    {
        throw std::runtime_error("Not implemented.");
    }
}

auto node_to_predicate_type(p_node const& node) -> predicate_type
{
    // TODO: Handle '\\' and '\"'

    if (node.is_type<predicate_less>())
    {
        p_node const& a1 = *node.children[0];
        p_node const& a2 = *node.children[1];

        if (a1.is_type<literal>())
        {
            if (!a2.is_type<identifier>())
            {
                throw std::runtime_error("Not implemented.");
            }

            return P_C_LESS_A{a2.string(), literal_node_to_value(*a1.children.front())};
        }
        else if (a2.is_type<literal>())
        {
            return P_A_LESS_C{a1.string(), literal_node_to_value(*a2.children.front())};
        }
        else
        {
            throw std::runtime_error("Not implemented.");
        }
    }
    else if (node.is_type<predicate_greater>())
    {
        p_node const& a1 = *node.children[0];
        p_node const& a2 = *node.children[1];

        if (a1.is_type<literal>())
        {
            if (!a2.is_type<identifier>())
            {
                throw std::runtime_error("Not implemented.");
            }

            return P_C_GREATER_A{a2.string(), literal_node_to_value(*a1.children.front())};
        }
        else if (a2.is_type<literal>())
        {
            return P_A_GREATER_C{a1.string(), literal_node_to_value(*a2.children.front())};
        }
        else
        {
            throw std::runtime_error("Not implemented.");
        }
    }
    else if (node.is_type<predicate_equal>())
    {
        p_node const& a1 = *node.children[0];
        p_node const& a2 = *node.children[1];

        if (a1.is_type<literal>())
        {
            if (!a2.is_type<identifier>())
            {
                throw std::runtime_error("Not implemented.");
            }

            return P_C_EQUAL_A{a2.string(), literal_node_to_value(*a1.children.front())};
        }
        else if (a2.is_type<literal>())
        {
            return P_A_EQUAL_C{a1.string(), literal_node_to_value(*a2.children.front())};
        }
        else
        {
            throw std::runtime_error("Not implemented.");
        }
    }
    else if (node.is_type<predicate_unequal>())
    {
        p_node const& a1 = *node.children[0];
        p_node const& a2 = *node.children[1];

        if (a1.is_type<literal>())
        {
            if (!a2.is_type<identifier>())
            {
                throw std::runtime_error("Not implemented.");
            }

            return P_C_UNEQUAL_A{a2.string(), literal_node_to_value(*a1.children.front())};
        }
        else if (a2.is_type<literal>())
        {
            return P_A_UNEQUAL_C{a1.string(), literal_node_to_value(*a2.children.front())};
        }
        else
        {
            throw std::runtime_error("Not implemented.");
        }
    }
    else if (node.is_type<predicate_between>())
    {
        p_node const& a1 = *node.children[0];
        p_node const& a2 = *node.children[1];
        p_node const& a3 = *node.children[2];

        return P_A_BETWEEN_C1_C2{
            a1.string(),
            literal_node_to_value(*a2.children.front()),
            literal_node_to_value(*a3.children.front())};
    }
    else
    {
        throw std::runtime_error("Predicate not implemented");
    }
}

auto process_select(p_node const& select_node) -> SelectExpression
{
    p_node const& columns = *select_node.children[0];
    assert(columns.is_type<select_columns>());

    p_node const& relation = *select_node.children[1];
    assert(relation.is_type<select_relation>());

    SelectExpression ret;

    for (auto const& it : columns.children)
    {
        assert(it->is_type<select_column>());
        ret.attributes.push_back(it->string());
    }

    ret.relation = relation.string();

    if (select_node.children.size() > 2)
    {
        p_node const& where_predicates = *select_node.children[2];

        assert(where_predicates.is_type<select_where>());

        for (auto const& predicate_it : where_predicates.children)
        {
            ret.predicates.emplace_back(node_to_predicate_type(*predicate_it));
        }
    }

    return ret;
}

auto process_create_table(p_node const& ct_node) -> CreateTableExpression
{
    p_node const& t_name = *ct_node.children[0];
    assert(t_name.is_type<table_name>());

    p_node const& attributes = *ct_node.children[1];
    assert(attributes.is_type<column_defs>());

    CreateTableExpression ret;
    ret.name = t_name.string();

    for (auto const& it : attributes.children)
    {
        p_node const& a_name = *it->children[0];
        p_node const& a_type = *it->children[1];

        if (it->children.size() > 2)
        {
            p_node const& a_primary_key = *it->children[2];
            assert(a_primary_key.is_type<primary_key>());

            if (ret.primary_key.has_value())
            {
                throw std::runtime_error("Two primary keys in definition");
            }

            ret.primary_key = a_name.string();
        }

        if (a_type.is_type<type_integer>())
        {
            ret.attributes.emplace_back(a_name.string(), INTEGER{});
        }
        else if (a_type.is_type<type_varchar>())
        {
            uint64_t n_chars = std::stoull(a_type.children[0]->string());
            ret.attributes.emplace_back(a_name.string(), VARCHAR{n_chars});
        }
        else
        {
            throw std::runtime_error("Not implemented");
        }
    }

    return ret;
}

auto process_tree(p_node const& node) -> ParsedExpression
{
    if (!node.is_root())
    {
        throw std::runtime_error("Root must be passed");
    }

    assert(node.children.size() > 0);
    p_node const& exp = *node.children.front();

    if (exp.is_type<s_select>())
    {
        return {process_select(exp)};
    }
    else if (exp.is_type<s_create_table>())
    {
        return {process_create_table(exp)};
    }
    else
    {
        throw std::runtime_error("Not implemented.");
    }
}
