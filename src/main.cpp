#include <iostream>

#include "attribute.hpp"
#include "database.hpp"

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

    fr.insert({11, "V_A2", "V_A3"});

    return 0;
}
