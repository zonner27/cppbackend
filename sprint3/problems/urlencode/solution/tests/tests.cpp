#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
}

TEST(UrlEncodeTestSuite, SpaceIsEncodedAsPlus) {
    EXPECT_EQ(UrlEncode("hello world"sv), "hello+world"s);
}

TEST(UrlEncodeTestSuite, ReservedCharsAreEncoded) {
    EXPECT_EQ(UrlEncode("!#$&'()*+,/:;=?@[]"sv), "%21%23%24%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D"s);
}

TEST(UrlEncodeTestSuite, NonAsciiCharsAreEncoded) {
    EXPECT_EQ(UrlEncode("\x80\xFF"sv), "%80%FF"s);
}

TEST(UrlEncodeTestSuite, MixedString) {
    EXPECT_EQ(UrlEncode("Hello World! This is a test."sv), "Hello+World%21+This+is+a+test."s);
}

TEST(UrlEncodeTestSuite, EmptyString) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
}

TEST(UrlEncodeTestSuite, PartialEncoding) {
    EXPECT_EQ(UrlEncode("abc*def"sv), "abc%2Adef"s);
}

//TEST(UrlEncodeTestSuite, AllPrintableAsciiChars) {
//    std::string all_printable_ascii =
//        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
//    std::string expected_encoding =
//        "+%21%22%23%24%25%26%27%28%29%2A%2B%2C-.%2F0123456789%3A%3B%3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E";
//    EXPECT_EQ(UrlEncode(all_printable_ascii), expected_encoding);
//}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
