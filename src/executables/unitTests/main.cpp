#define CATCH_CONFIG_MAIN //Make main function automatically
#include "catch.hpp"

#include "exampleHeaderFile.hpp"

TEST_CASE("Example test case", "[Example]")
{
SECTION("Example section", "[Example]")
{
int testInt = 5;
double testDouble = 10.0;
std::string testString = "ABC";

REQUIRE(testInt == 5);
REQUIRE(testDouble == Approx(10.0));
REQUIRE(testString == "ABC");

REQUIRE((testInt + testDouble) == Approx(15.0));

REQUIRE(thisFunctionConvertsToInt(5.9) == 5);
}
}

