#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    // Напишите остальные тесты для функции UrlDecode самостоятельно

    BOOST_TEST(UrlDecode("HelloWorld"s) == "HelloWorld"s);

    BOOST_TEST(UrlDecode("Hello%20World%21"s) == "Hello World!"s);
    BOOST_TEST(UrlDecode("Hello%20World%21%3F"s) == "Hello World!?"s);
    BOOST_TEST(UrlDecode("Hello%20World%21%3f"s) == "Hello World!?"s);
    BOOST_TEST(UrlDecode("%68%65%6C%6C%6F"s) == "hello"s);

    // Строка с невалидными %-последовательностями
    BOOST_CHECK_THROW(UrlDecode("Hello%2XWorld"s), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("Hello%World"s), std::invalid_argument);

    // Строка с неполными %-последовательностями
    BOOST_CHECK_THROW(UrlDecode("Hello%"s), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("Hello%2"s), std::invalid_argument);

    // Строка с символом +
    BOOST_TEST(UrlDecode("Hello+World"s) == "Hello World"s);
    BOOST_TEST(UrlDecode("+Hello+World+"s) == " Hello World "s);

    // Смешанные случаи
    BOOST_TEST(UrlDecode("Hello+%20World%21"s) == "Hello  World!"s);
    BOOST_TEST(UrlDecode("%48%65%6C%6C%6F+World%21"s) == "Hello World!"s);

}
