#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/pqxx>
#include <iostream>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
            INSERT INTO authors (id, name) VALUES ($1, $2)
            ON CONFLICT (id) DO UPDATE SET name=$2;
        )"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

std::vector<domain::Author> AuthorRepositoryImpl::GetAllAuthors() {
    std::vector<domain::Author> authors;

    pqxx::read_transaction read_transaction_{connection_};

    auto result = read_transaction_.exec("SELECT id, name FROM authors ORDER BY name ASC");

    for (const auto& row : result) {
        std::string id = row[0].as<std::string>();
        std::string name = row[1].as<std::string>();
        authors.emplace_back(domain::AuthorId::FromString(id), name);
    }

    return authors;
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    pqxx::work work_{connection_};
    //std::string title = book.GetTitle();
    work_.exec_params(R"(
    INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
        ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
        )"_zv,
        book.GetId().ToString(),
        book.GetAuthorId().ToString(),
        book.GetTitle(),
        book.GetPublicationYear())
    ;
    work_.commit();
}

std::vector<domain::Book> BookRepositoryImpl::GetBooksBy(const std::string& author_id_str) {
    std::vector<domain::Book> books;
    pqxx::read_transaction read_transaction_{connection_};
    auto query_text = "SELECT id, author_id, title, publication_year FROM books WHERE author_id = "
            + read_transaction_.quote(author_id_str)
            + " ORDER BY publication_year ASC, title ASC";
    for (auto [id, author_id, title, year] : read_transaction_.query<std::string, std::string, std::string, int>(query_text)) {
        books.emplace_back(
            domain::BookId::FromString(id),
            domain::AuthorId::FromString(author_id),
            title,
            year
        );
    }
    return books;
}


std::vector<domain::Book> BookRepositoryImpl::GetAllBooks() {
    std::vector<domain::Book> books;
    pqxx::read_transaction read_transaction_{connection_};
    auto query_text = "SELECT id, author_id, title, publication_year FROM books ORDER BY title ASC"_zv;
    for (auto [id, author_id, title, year] : read_transaction_.query<std::string, std::string, std::string, int>(query_text)) {
        books.emplace_back(
            domain::BookId::FromString(id),
            domain::AuthorId::FromString(author_id),
            title,
            year
        );
    }
    return books;
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};

    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);

    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL,
    title varchar(100) NOT NULL,
    publication_year int NOT NULL
);
)"_zv);

    // ... создать другие таблицы

    // коммитим изменения
    work.commit();
}

}  // namespace postgres
