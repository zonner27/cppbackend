#pragma once
#include <string>
#include <vector>
#include <optional>

#include "../util/tagged_uuid.h"
#include "author.h"
#include "../ui/view.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
public:
    Book(BookId id, AuthorId author_id, std::string title, int year)
        : id_(std::move(id))
        , author_id_(std::move(author_id))
        , title_(std::move(title))
        , publication_year_(year)  {
    }

    const BookId& GetId() const noexcept;
    const AuthorId& GetAuthorId() const noexcept;
    const std::string& GetTitle() const noexcept;
    int GetPublicationYear() const noexcept;

private:
    BookId id_;
    AuthorId author_id_;
    std::string title_;
    int publication_year_;
};

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual void Delete(const std::string& id) = 0;
    virtual std::vector<Book> GetBooksBy(const std::string& author_id) = 0;
    virtual std::vector<Book> GetAllBooks() = 0;
    virtual std::vector<ui::detail::BookInfoAllBooks> GetAllBooksForShow() = 0;
    virtual std::vector<ui::detail::BookInfoAllBooks> GetBooksBytitle(const std::string& title) = 0;
    virtual ui::detail::BookInfoAllBooks GetBook(const std::string& id) = 0;
    virtual void UpdateBook(const std::string& book_id,
                            const std::string& new_title, int new_year) = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
