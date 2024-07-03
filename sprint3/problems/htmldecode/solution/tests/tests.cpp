#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
}

TEST_CASE("Text with HTML mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("M&amp;M&APOSs"sv) == "M&M's"s);
    CHECK(HtmlDecode("Johnson&amp;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == "Johnson&Johnson"s);
}

TEST_CASE("Text with mnemonics in different cases", "[HtmlDecode]") {
    CHECK(HtmlDecode("&LT;&GT;&AMP;&APOS;&QUOT;"sv) == "<>&'\""s);
    CHECK(HtmlDecode("&lt;&gt;&amp;&apos;&quot;"sv) == "<>&'\""s);
}

TEST_CASE("Text with mixed case mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&Lt;&Gt;&aMp;&aPoS;&qUoT;"sv) == "<>&'\""s);
}

TEST_CASE("Text with mnemonics at different positions", "[HtmlDecode]") {
    CHECK(HtmlDecode("&amp;hello"sv) == "&hello"s);
    CHECK(HtmlDecode("hello&amp;"sv) == "hello&"s);
    CHECK(HtmlDecode("he&amp;llo"sv) == "he&llo"s);
}

TEST_CASE("Text with incomplete mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&"sv) == "&"s);
    CHECK(HtmlDecode("&amp"sv) == "&"s);
    CHECK(HtmlDecode("&apo"sv) == "&apo"s);
}

//TEST_CASE("Text with mnemonics with and without semicolon", "[HtmlDecode]") {
//    CHECK(HtmlDecode("&lt;&gt;&amp;&apos;&quot;"sv) == "<>&'\""s);
//    CHECK(HtmlDecode("&lt&gt&amp&apos&quot"sv) == "<>&'\""s);
//}
