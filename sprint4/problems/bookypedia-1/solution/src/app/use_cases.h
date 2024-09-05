#pragma once

#include <string>
#include <vector>
#include <optional>
#include "../ui/view.h"


namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual std::vector<ui::detail::AuthorInfo> GetAllAuthors() = 0;
    virtual void AddBook(const std::string& author_id, const std::string& title, int year) = 0;
    virtual std::vector<ui::detail::BookInfo> GetBooksBy(const std::string& author_id) = 0;
    virtual std::vector<ui::detail::BookInfo> GetAllbooks() = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
