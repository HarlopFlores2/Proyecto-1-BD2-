#pragma once

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

using ParsedExpression = std::variant<SelectExpression, CreateTableExpression>;

auto literal_node_to_value(pegtl::parse_tree::node const& node) -> nlohmann::json;
auto node_to_predicate_type(pegtl::parse_tree::node const& node) -> predicate_type;

auto process_tree(pegtl::parse_tree::node const& node) -> ParsedExpression;
