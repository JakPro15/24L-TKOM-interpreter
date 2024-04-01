#include "lexer.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dummy", "[Lexer]")
{
    auto l = Lexer();
    REQUIRE(l.dummy() == 42);
}
