#include <cassert>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "json.hpp"
#include "predicates.hpp"

#include "tao/pegtl.hpp"
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>

#include "prettyprint.hpp"
#include "tao/pegtl/string_input.hpp"

using json = nlohmann::json;

namespace pegtl = tao::pegtl;

// clang-format off
struct spaces_p : pegtl::plus<pegtl::space> {};
struct spaces_s : pegtl::star<pegtl::space> {};

struct k_and : pegtl::istring<'a', 'n', 'd'> {};
struct k_between : pegtl::istring<'b', 'e', 't', 'w', 'e', 'e', 'n'> {};
struct k_create : pegtl::istring<'c', 'r', 'e', 'a', 't', 'e'> {};
struct k_from : pegtl::istring<'f', 'r', 'o', 'm'> {};
struct k_select : pegtl::istring<'s', 'e', 'l', 'e', 'c', 't'> {};
struct k_table : pegtl::istring<'t', 'a', 'b', 'l', 'e'> {};
struct k_where : pegtl::istring<'w', 'h', 'e', 'r', 'e'> {};
struct k_semicolon : pegtl::one<';'> {};

struct identifier : pegtl::seq<pegtl::alpha, pegtl::star<pegtl::alnum>> {};

struct lit_number :
    pegtl::seq<pegtl::opt<pegtl::one<'+', '-'>>,
               pegtl::plus<pegtl::digit>>
{};
struct escaped_double_quotes : pegtl::string<'\\', '"'> {};
struct escaped_backslash : pegtl::string<'\\', '\\'> {};
struct character : pegtl::sor<escaped_backslash,
                              escaped_double_quotes,
                              pegtl::not_one<'"'>>
{};
struct lit_string : pegtl::seq<
  pegtl::star<character>
> {};
struct lit_string_quotes : pegtl::seq<
  pegtl::one<'"'>,
    lit_string,
  pegtl::one<'"'>
> {};
struct literal : pegtl::sor<lit_number, lit_string_quotes> {};

struct select_column : pegtl::sor<pegtl::one<'*'>, identifier> {};
struct select_columns : pegtl::list<select_column, pegtl::one<','>, pegtl::space> {};
struct select_relation : identifier {};

struct identifier_or_literal : pegtl::sor<identifier, literal> {};

struct predicate_less :
    pegtl::seq<identifier_or_literal,
               spaces_s,
               pegtl::one<'<'>,
               spaces_s,
               identifier_or_literal> {};
struct predicate_greater :
    pegtl::seq<identifier_or_literal,
               spaces_s,
               pegtl::one<'<'>,
               spaces_s,
               identifier_or_literal> {};
struct predicate_equal :
    pegtl::seq<identifier_or_literal,
               spaces_s,
               pegtl::string<'=', '='>,
               spaces_s,
               identifier_or_literal> {};
struct predicate_unequal :
    pegtl::seq<identifier_or_literal,
               spaces_s,
               pegtl::one<'!', '='>,
               spaces_s,
               identifier_or_literal> {};
struct predicate_between :
    pegtl::seq<identifier,
               spaces_p,
               k_between,
               spaces_p,
               literal,
               spaces_p,
               k_and,
               spaces_p,
               literal> {};

struct predicate :
    pegtl::sor<
        predicate_less,
        predicate_greater,
        predicate_equal,
        predicate_unequal,
        predicate_between>
{};

struct select_where : pegtl::seq<
    k_where,
    spaces_p,
    predicate,
    pegtl::star<
        pegtl::seq<
            spaces_p,
            k_and,
            spaces_p,
            predicate
        >
    >
> {};

struct s_select : pegtl::seq<
    k_select,
    spaces_p,
    select_columns,
    spaces_p,
    k_from,
    spaces_p,
    select_relation,
    pegtl::opt<pegtl::seq<spaces_p, select_where>>,
    spaces_s,
    k_semicolon
>
{};

struct grammar : pegtl::must<s_select> {};

// clang-format on

template<typename Rule>
using selector = pegtl::parse_tree::selector<
    Rule,
    pegtl::parse_tree::store_content::on<
        identifier,
        literal,
        lit_number,
        lit_string,

        select_columns,
        select_column,
        select_relation,
        s_select,
        select_where,
        predicate_less,
        predicate_greater,
        predicate_equal,
        predicate_unequal,
        predicate_between>,
    pegtl::parse_tree::fold_one::on<identifier_or_literal>>;

struct SelectExpression
{
    std::vector<std::string> attributes;
    std::vector<predicate_type> predicates;
    std::string relation;
};

struct CreateTableExpression
{
    std::string name;
    std::vector<std::string> attributes;
    std::vector<std::string> primary_key;
};

using Expression = std::variant<SelectExpression, CreateTableExpression>;

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

auto process_tree(p_node const& node) -> Expression
{
    if (!node.is_root())
    {
        throw std::runtime_error("Root must be passed");
    }

    assert(node.children.size() > 0);
    p_node const& exp = *node.children.front();

    if (exp.is_type<s_select>())
    {
        p_node const& columns = *exp.children[0];
        assert(columns.is_type<select_columns>());

        p_node const& relation = *exp.children[1];
        assert(relation.is_type<select_relation>());

        SelectExpression ret;

        for (auto const& it : columns.children)
        {
            assert(it->is_type<select_column>());
            ret.attributes.push_back(it->string());
        }

        ret.relation = relation.string();

        if (exp.children.size() > 2)
        {
            p_node const& where_predicates = *exp.children[2];

            assert(where_predicates.is_type<select_where>());

            for (auto const& predicate_it : where_predicates.children)
            {
                ret.predicates.emplace_back(node_to_predicate_type(*predicate_it));
            }
        }

        return {ret};
    }
    else
    {
        throw std::runtime_error("Not implemented.");
    }
}

auto main() -> int
{
    std::string s1 = R"(select * from r WHERE a == 99 AND b < "hello";)";
    pegtl::string_input in1(s1, "s1");
    auto r1 = pegtl::parse_tree::parse<grammar, selector>(in1);

    std::boolalpha(std::cerr);
    std::cerr << bool(r1) << "\n";

    auto ret = process_tree(*r1);

    // SelectExpression se;

    // std::string s = R"(select *, a1, a2 from r WHERE a1 == 99;)";
    // std::string s = R"(select *, a1, a2 from r WHERE a1 == 99 AND a2 < "hello" AND a3 >
    // 1823;)";
    // pegtl::string_input in(s, "s1");

    // pegtl::parse<grammar, action>(in, se);

    // std::cerr << se.attributes << "\n";
    // std::cerr << se.relation << "\n";

    // auto root = pegtl::parse_tree::parse<grammar, selector>(in);

    // std::cerr << bool(root) << "\n";

    // auto pt = process_tree(*root);

    pegtl::parse_tree::print_dot(std::cerr, *r1);
}
