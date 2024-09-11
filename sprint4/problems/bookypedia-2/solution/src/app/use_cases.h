#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_set>
#include <set>
#include "../ui/view.h"


namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual void DeleteAuthor(const std::string& id) = 0;
    virtual void EditAuthor(const std::string& id, const std::string& new_name) = 0;
    virtual std::vector<ui::detail::AuthorInfo> GetAllAuthors() = 0;
    virtual ui::detail::BookInfo AddBook(const std::string& author_id, const std::string& title, int year) = 0;
    virtual void DeleteBook(const std::string& id) = 0;
    virtual std::vector<ui::detail::BookInfo> GetBooksBy(const std::string& author_id) = 0;
    virtual std::vector<ui::detail::BookInfo> GetAllbooks() = 0;
    virtual std::vector<ui::detail::BookInfoAllBooks> GetAllBooksForShow() = 0;
    virtual std::vector<ui::detail::BookInfoAllBooks> GetBooksByTitle(const std::string title) = 0;
    virtual ui::detail::BookInfoAllBooks GetBookByBookId(const std::string &id) = 0;
    virtual void AddTagsBooks(std::set<std::string>, const std::string& book_id) = 0;
    virtual std::optional<std::string> GetTagsByBookId(const std::string& title) = 0;
    virtual void UpdateBook(const std::string &id,
                    const std::string& new_title, int new_year) = 0;
    virtual void DeleteTagsByBookId(const std::string book_id) = 0 ;

protected:
    ~UseCases() = default;
};

}  // namespace app
