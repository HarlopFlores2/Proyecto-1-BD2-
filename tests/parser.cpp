#include "attribute.hpp"
#include "catch.hpp"

#include <iostream>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"
#include "tao/pegtl/contrib/parse_tree_to_dot.hpp"

#include "parser.hpp"
#include "tao/pegtl/string_input.hpp"

namespace pegtl = tao::pegtl;

TEST_CASE("Parser", "[parser]")
{
    std::string q =
        "CREATE TABLE T1 (A1 INTEGER, A2 INTEGER, A5 VARCHAR(99) PRIMARY KEY, A8 VARCHAR(10));";
    pegtl::string_input in(q, "q1");
    auto r = pegtl::parse_tree::parse<grammar, selector>(in);

    CHECK(bool(r));
    pegtl::parse_tree::print_dot(std::cerr, *r);

    auto cte_o = process_tree(*r);
    auto cte = std::get<CreateTableExpression>(cte_o);
    CHECK(cte.name == "T1");
    CHECK(
        cte.attributes
        == std::vector<Attribute>{
            {"A1",   INTEGER{}},
            {"A2",   INTEGER{}},
            {"A5", VARCHAR{99}},
            {"A8", VARCHAR{10}}
    });
    CHECK(cte.primary_key.has_value());
    CHECK(cte.primary_key.value() == "A5");
}
