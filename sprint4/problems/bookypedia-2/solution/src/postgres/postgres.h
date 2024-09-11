#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"


namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::Author& author) override;
    void Delete(const std::string& id) override;
    void UpdateName(const std::string& id, const std::string& new_name) override;
    std::vector<domain::Author> GetAllAuthors() override;


private:
    pqxx::connection& connection_;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection)
        : connection_(connection) {
    }

    void Save(const domain::Book& book) override;
    void Delete(const std::string& id) override;
    std::vector<domain::Book> GetBooksBy(const std::string& author_id) override;
    std::vector<domain::Book> GetAllBooks() override;
    std::vector<ui::detail::BookInfoAllBooks> GetAllBooksForShow() override;
    std::vector<ui::detail::BookInfoAllBooks> GetBooksBytitle(const std::string& title) override;
    ui::detail::BookInfoAllBooks GetBook(const std::string& id) override;
    void UpdateBook(const std::string& book_id, const std::string& new_title, int new_year) override;

private:
    pqxx::connection& connection_;
};

class TagRepositoryImpl : public domain::TagRepository {
public:
    explicit TagRepositoryImpl(pqxx::connection& connection)
        : connection_(connection) {
    }

    void Save(const domain::Tag& tag) override;
    std::optional<std::string> GetTagsByBookId(const std::string book_id) override;
    void DeleteTagsByBookId(const std::string book_id) override;
;


private:
    pqxx::connection& connection_;
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    AuthorRepositoryImpl& GetAuthors() & {
        return authors_;
    }

    BookRepositoryImpl& GetBooks() & {
        return books_;
    }

    TagRepositoryImpl& GetTags() & {
        return tags_;
    }

private:
    pqxx::connection connection_;
    AuthorRepositoryImpl authors_{connection_};
    BookRepositoryImpl books_{connection_};
    TagRepositoryImpl tags_{connection_};
};

}  // namespace postgres
