#include "catch.hpp"

#include "attribute.hpp"

TEST_CASE("Potpourri", "[CHANGEME]")
{
    Attribute a1{"A1", "INTEGER"};
    Attribute a2{"A2", "VARCHAR(10)"};
    Attribute a3{"A2", "VARCHAR(24)"};

    CHECK(a1.to_specifier() == "INTEGER");
    CHECK(a2.to_specifier() == "VARCHAR(10)");
    CHECK(a3.to_specifier() == "VARCHAR(24)");
}
