#include "urldecode.h"

#include <charconv>
#include <stdexcept>
#include <stdexcept>

std::string UrlDecode(std::string_view url) {
    std::string result;
    result.reserve(url.size());
    for (size_t i = 0; i < url.size(); ++i) {
        if (url[i] == '%') {
            if (i + 2 < url.size()) {
                std::string hex(url.substr(i + 1, 2));
                if (isxdigit(hex[0]) && isxdigit(hex[1])) {
                    char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
                    result += decoded_char;
                    i += 2;
                } else {
                    throw std::invalid_argument("Invalid percent-encoded sequence in URL");
                }
            } else {
                throw std::invalid_argument("Incomplete percent-encoded sequence in URL");
            }
        } else if (url[i] == '+') {
            result += ' ';
        } else {
            result += url[i];
        }
    }
    return result;
}

//std::string UrlDecode(std::string_view url) {
//    // Реализуйте функцию UrlDecode самостоятельно
//    std::string result;
//    result.reserve(url.size());
//    for (size_t i = 0; i < url.size(); ++i) {
//        if (url[i] == '%') {
//            if (i + 2 < url.size()) {
//                std::string hex(url.substr(i + 1, 2));
//                char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
//                result += decoded_char;
//                i += 2;
//            }
//        } else if (url[i] == '+') {
//            result += ' ';
//        } else {
//            result += url[i];
//        }
//    }
//    return result;
//}
