#include "urlencode.h"

#include <string_view>
#include <iomanip>
#include <sstream>


std::string UrlEncode(std::string_view str) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (char ch : str) {
        if (isalnum(static_cast<unsigned char>(ch)) ||
            ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            // Алфавитно-цифровые символы и некоторые специальные символы остаются неизменными
            encoded << ch;
        } else if (ch == ' ') {
            // Пробел кодируется как +
            encoded << '+';
        } else {
            // Все прочие символы кодируются в формате %xx
            encoded << '%' << std::setw(2) << std::uppercase << int(static_cast<unsigned char>(ch));
        }
    }

    return encoded.str();
}
