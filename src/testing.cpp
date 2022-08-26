#include "./utils.h"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("word_distance are computed", "[word_distance]")
{
    REQUIRE(utils::word_distance("hello", "world", true) == 4);
}
