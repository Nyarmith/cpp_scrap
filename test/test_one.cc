#include "catch2/catch.hpp"

TEST_CASE("Statement of test", "[testname]") {
    SECTION("Initialize") {
        SECTION("Using Correct IP") {
            // do stuff
            REQUIRE();
        }

        SECTION(""){
            // stuff
            REQUIRE(/*boolean-condition*/);
        }
        WARN("thing")
        INFO("info")
    }
}
