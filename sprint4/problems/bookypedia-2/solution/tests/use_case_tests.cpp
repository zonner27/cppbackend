#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"
#include "../src/domain/tag.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }

    void Delete(const std::string& id) override {

    }

    void UpdateName(const std::string& id, const std::string& new_name) override{

    }

    std::vector<domain::Author> GetAllAuthors() override {
        std::vector<domain::Author> authors;
        return authors;
    }
};

struct MockBookRepository : domain::BookRepository {
    std::vector<domain::Book> saved_book;

    void Save(const domain::Book& book) override {
        saved_book.emplace_back(book);
    }
    void Delete(const std::string& id) override {

    }
    std::vector<domain::Book> GetBooksBy(const std::string& author_id) override {
        std::vector<domain::Book> books;

        return books;
    }
    std::vector<domain::Book> GetAllBooks() override{
        std::vector<domain::Book> books;

        return books;
    }
    std::vector<ui::detail::BookInfoAllBooks> GetAllBooksForShow() override {
        std::vector<ui::detail::BookInfoAllBooks> books_info;

        return books_info;
    }
    std::vector<ui::detail::BookInfoAllBooks> GetBooksBytitle(const std::string& title) override {
        std::vector<ui::detail::BookInfoAllBooks> books_info;

        return books_info;
    }
    ui::detail::BookInfoAllBooks GetBook(const std::string& id) override {
        ui::detail::BookInfoAllBooks book_info;

        return book_info;
    }
    void UpdateBook(const std::string& book_id,
                    const std::string& new_title, int new_year) override {

    }
};

struct MockTagRepository : domain::TagRepository {
    std::vector<domain::Tag> saved_tag;
    void Save(const domain::Tag& tag) override {
        saved_tag.emplace_back(tag);
    }
    std::optional<std::string> GetTagsByBookId(const std::string book_id) override{
        return std::nullopt;
    }
    void DeleteTagsByBookId(const std::string book_id) override{

    }
};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
    MockTagRepository tags;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases(authors, books, tags);

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(authors.saved_authors.at(0).GetId() != domain::AuthorId{});
            }
        }
    }
}
