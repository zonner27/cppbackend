#include "htmldecode.h"

#include <string_view>
#include <unordered_map>
#include <algorithm>


std::string HtmlDecode(std::string_view str) {
    static const std::unordered_map<std::string, char> html_entities = {
        {"lt", '<'},
        {"gt", '>'},
        {"amp", '&'},
        {"apos", '\''},
        {"quot", '"'}
    };

    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '&') {
            size_t semi_pos = str.find(';', i);
            size_t len = (semi_pos != std::string::npos) ? (semi_pos - i) : 5;

            std::string potential_entity = std::string(str.substr(i + 1, len - 1));
            std::string potential_entity_without_semicolon = std::string(str.substr(i + 1, len - 1));

            // Convert to lowercase
            std::transform(potential_entity.begin(), potential_entity.end(), potential_entity.begin(), ::tolower);
            std::transform(potential_entity_without_semicolon.begin(), potential_entity_without_semicolon.end(), potential_entity_without_semicolon.begin(), ::tolower);

            if (html_entities.count(potential_entity) && semi_pos != std::string::npos) {
                result += html_entities.at(potential_entity);
                i = semi_pos;
            } else if (html_entities.count(potential_entity_without_semicolon)) {
                result += html_entities.at(potential_entity_without_semicolon);
                i += len - 1;
            } else {
                result += '&';
            }
        } else {
            result += str[i];
        }
    }

    return result;
}
