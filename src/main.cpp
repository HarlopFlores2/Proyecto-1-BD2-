#include <iostream>

#include "attribute.hpp"
#include "database.hpp"
#include "operations.hpp"
#include "parser.hpp"
#include "predicates.hpp"

#include "tao/pegtl.hpp"
#include "tao/pegtl/string_input.hpp"

namespace pegtl = tao::pegtl;

auto parse_expression(std::string const& s) -> ParsedExpression
{
    pegtl::string_input in(s, "String parsed");

    auto root = pegtl::parse_tree::parse<grammar, selector>(in);
    if (!root)
    {
        throw std::runtime_error("Expression was not valid.");
    }

    return process_tree(*root);
}

auto main() -> int
{
    DataBase db("D1");

    auto& fr = db.create_relation(
        "R1",
        {
            {"A1",   INTEGER{}},
            {"A2", VARCHAR{10}},
            {"A3", VARCHAR{24}}
    },
        "A1");

    fr.insert({11, "A2_1", "A3_1"});
    fr.insert({22, "A2_2", "A3_2"});
    fr.insert({33, "A2_3", "A3_3"});
    fr.insert({44, "A2_4", "A3_4"});

    relation_to_csv(db.project("R1", {"A3", "A2", "A1"}), std::cerr);

    relation_to_csv(
        db.select(
            "R1",
            {
                P_C_UNEQUAL_A{"A1", 99}
    }),
        std::cerr);
    relation_to_csv(
        project(
            db.select(
                "R1",
                {
                    P_C_UNEQUAL_A{"A1", 33}
    }),
            {"A1"}),
        std::cerr);

    relation_to_csv(
        db.evaluate(parse_expression(R"(SELECT * FROM R1 WHERE A3 == "A3_1";)")).value(),
        std::cerr);

    return 0;
}
