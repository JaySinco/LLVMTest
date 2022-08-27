#include "../utils.h"
#include <catch2/catch.hpp>

TEST_CASE("sort")
{
    SECTION("stub model") { CHECK(utils::word_distance("hello", "world", true) == 4); }
}