#include <iostream>
#include <istream>
#include <string>

#include "database.hpp"
#include "operations.hpp"
#include "parser.hpp"
#include "predicates.hpp"

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"
#include "tao/pegtl/contrib/parse_tree_to_dot.hpp"

namespace pegtl = tao::pegtl;

void p_dot(pegtl::parse_tree::node const& n)
{
    pegtl::parse_tree::print_dot(std::cerr, n);
}

auto parse_expression(std::string const& s) -> ParsedExpression
{
    // FIXME: For some reason some data in the trees resulting from this function are being
    // deleted resulting in the processing failing, I have no clue as to why. It doesn't seem to
    // happen in our main. I SPOKE TOO SOON

    pegtl::string_input in(s, "String parsed");

    auto root = pegtl::parse_tree::parse<grammar, selector>(in);
    if (!root)
    {
        throw std::runtime_error("Expression was not valid.");
    }

    return process_tree(*root);
}

auto main(int argc, char** argv) -> int
{
    DataBase db("D1");

    if (argc == 1)
    {
        std::string temp;
        while (std::getline(std::cin, temp, ';'))
        {
            temp += ";";
            auto mr_o = db.evaluate(parse_expression(temp));

            if (mr_o.has_value())
            {
                relation_to_csv(mr_o.value(), std::cout);
            }
            else
            {
                std::cout << "No output" << std::endl;
            }

            std::cin >> std::ws;
        }

        return 0;
    }

    pegtl::argv_input argv_in(argv, 1);

    auto root = pegtl::parse_tree::parse<grammar, selector>(argv_in);
    if (!root)
    {
        throw std::runtime_error("Expression was not valid.");
    }

    if (argc == 3)
    {
        pegtl::parse_tree::print_dot(std::cout, *root);
    }
    else
    {
        ParsedExpression pe = process_tree(*root);

        auto mr_o = db.evaluate(pe);

        if (mr_o.has_value())
        {
            relation_to_csv(mr_o.value(), std::cout);
        }
        else
        {
            std::cout << "No output\n";
        }
    }

    return 0;
}
