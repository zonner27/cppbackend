#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include <unordered_set>
#include <set>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string id;
    std::string title;
    int publication_year;
};

struct BookInfoAllBooks {
    std::string id;
    std::string title;
    std::string author_name;
    int publication_year;
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool AddBook(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;
    bool AddTags(std::set<std::string>& tags, std::string &book_id) const;
    bool ShowBooks() const;
    bool ShowBook(std::istream& cmd_input) const;
    bool ShowAuthorBooks() const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::optional<std::string> SelectAuthor(std::vector<detail::AuthorInfo>& authors) const;
    std::optional<std::string> SelectBook() const;
    std::optional<std::string> SelectBookFromBooks(std::vector<ui::detail::BookInfoAllBooks> books) const;
    std::vector<detail::AuthorInfo> GetAuthors() const;
    std::optional<std::string> GetAuthorsIdByAuthorName(const std::string& author_name) const;
    detail::BookInfoAllBooks GetBookByBookId(const std::string& id) const;
    std::vector<detail::BookInfo> GetBooks() const;
    std::vector<detail::BookInfoAllBooks> GetAllBooksForShow() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;

    void printBook(const ui::detail::BookInfoAllBooks& book) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui
