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

void AuthorRepositoryImpl::Delete(const std::string &id) {
    pqxx::work work{connection_};

    bool book_tags_exists = false;
    bool books_exists = false;
    bool authors_exists = false;

    pqxx::result result;

    result = work.exec(R"(
        SELECT EXISTS (
            SELECT 1
            FROM information_schema.tables
            WHERE table_schema = 'public'
            AND table_name = 'book_tags'
        );
    )");
    if (result[0][0].as<bool>()) {
        book_tags_exists = true;
    }

    result = work.exec(R"(
        SELECT EXISTS (
            SELECT 1
            FROM information_schema.tables
            WHERE table_schema = 'public'
            AND table_name = 'books'
        );
    )");
    if (result[0][0].as<bool>()) {
        books_exists = true;
    }

    result = work.exec(R"(
        SELECT EXISTS (
            SELECT 1
            FROM information_schema.tables
            WHERE table_schema = 'public'
            AND table_name = 'authors'
        );
    )");
    if (result[0][0].as<bool>()) {
        authors_exists = true;
    }

    if (book_tags_exists) {
        work.exec_params(R"(
            DELETE FROM book_tags
            WHERE book_id IN (SELECT id FROM books WHERE author_id = $1);
        )"_zv, id);
    }

    if (books_exists) {
        work.exec_params(R"(
            DELETE FROM books
            WHERE author_id = $1;
        )"_zv, id);
    }

    if (authors_exists) {
        work.exec_params(R"(
            DELETE FROM authors
            WHERE id = $1;
        )"_zv, id);
    }

    work.commit();
}

void AuthorRepositoryImpl::UpdateName(const std::string &id, const std::string &new_name) {
    pqxx::work work{connection_};
    work.exec_params(R"(
        UPDATE authors
        SET name = $1
        WHERE id = $2;
    )"_zv, new_name, id);

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
    pqxx::work work{connection_};
    work.exec_params(R"(
    INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
        ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
        )"_zv,
        book.GetId().ToString(),
        book.GetAuthorId().ToString(),
        book.GetTitle(),
        book.GetPublicationYear())
    ;
    work.commit();
}

void BookRepositoryImpl::Delete(const std::string &id) {
    pqxx::work work{connection_};

    work.exec_params(R"(
        DELETE FROM book_tags WHERE book_id = $1;
    )"_zv, id);

    work.exec_params(R"(
        DELETE FROM books WHERE id = $1;
    )"_zv, id);

    work.commit();
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

std::vector<ui::detail::BookInfoAllBooks> BookRepositoryImpl::GetAllBooksForShow() {

    std::vector<ui::detail::BookInfoAllBooks> books;
    pqxx::read_transaction read_transaction_{connection_};

    // Объединяем книги и авторов, сортируем по названию книги, имени автора и году публикации
    auto query_text = R"(
        SELECT books.id, books.title, authors.name, books.publication_year
        FROM books
        JOIN authors ON books.author_id = authors.id
        ORDER BY books.title ASC, authors.name ASC, books.publication_year ASC
    )"_zv;

    for (auto [id, title, author_name, year] :
         read_transaction_.query<std::string, std::string, std::string, int>(query_text)) {
        books.emplace_back(ui::detail::BookInfoAllBooks{
            id,
            title,
            author_name,
            year
        });
    }
    return books;
}

std::vector<ui::detail::BookInfoAllBooks> BookRepositoryImpl::GetBooksBytitle(const std::string &title){
    std::vector<ui::detail::BookInfoAllBooks> books;
    pqxx::read_transaction read_transaction_{connection_};

    auto query_text = R"(
        SELECT books.id, books.title, authors.name, books.publication_year
        FROM books
        JOIN authors ON books.author_id = authors.id
        WHERE books.title = $1
        ORDER BY books.title ASC, authors.name ASC, books.publication_year ASC
    )"_zv;

    for (auto row : read_transaction_.exec_params(query_text, title)) {
        std::string id = row[0].as<std::string>();
        std::string book_title = row[1].as<std::string>();
        std::string author_name = row[2].as<std::string>();
        int year = row[3].as<int>();

        books.emplace_back(ui::detail::BookInfoAllBooks{
            id,
            book_title,
            author_name,
            year
        });
    }

    return books;
}

ui::detail::BookInfoAllBooks BookRepositoryImpl::GetBook(const std::string &id) {
    pqxx::read_transaction read_transaction{connection_};

    auto query_text = R"(
        SELECT id, author_id, title, publication_year
        FROM books
        WHERE id = $1
    )"_zv;

    pqxx::result result = read_transaction.exec_params(query_text, id);

    if (result.empty()) {
        throw std::runtime_error("Book not found with id: " + id);
    }

    auto row = result[0];
    std::string book_id = row[0].as<std::string>();
    std::string author_id = row[1].as<std::string>();
    std::string title = row[2].as<std::string>();
    int publication_year = row[3].as<int>();

    return ui::detail::BookInfoAllBooks{
        book_id,
        title,
        author_id,
        publication_year
    };
}

void BookRepositoryImpl::UpdateBook(const std::string& book_id,
                                     const std::string& new_title, int new_year){
    pqxx::work work{connection_};
    work.exec_params(R"(
        UPDATE books
        SET title = $1,
        publication_year = $2
        WHERE id = $3;
    )"_zv, new_title, new_year, book_id);

    work.commit();
}

void TagRepositoryImpl::Save(const domain::Tag& tag_input) {
    pqxx::work work{connection_};

    work.exec_params(R"(
    INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)
        )"_zv,
        tag_input.GetBookId().ToString(),
        tag_input.GetTag())
    ;

    work.commit();
}

std::optional<std::string> TagRepositoryImpl::GetTagsByBookId(const std::string book_id){
    pqxx::read_transaction read_transaction_{connection_};

    auto query_text = R"(
        SELECT book_tags.tag
        FROM book_tags
        WHERE book_tags.book_id = $1
    )"_zv;

    pqxx::result result = read_transaction_.exec_params(query_text, book_id);
    if (result.empty()) {
        return std::nullopt;
    }

    std::string tags;
    for (const auto& row : result) {
        if (!tags.empty()) {
            tags += ", ";
        }
        tags += row["tag"].as<std::string>();
    }

    return tags;
}

void TagRepositoryImpl::DeleteTagsByBookId(const std::string book_id){
    pqxx::work work{connection_};
    work.exec_params(R"(
        DELETE FROM book_tags WHERE book_id = $1;
    )"_zv, book_id);

    work.commit();
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};

    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT firstindex PRIMARY KEY,
    name varchar(100) NOT NULL UNIQUE
);
)"_zv);

    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID PRIMARY KEY,
    author_id UUID,
    title VARCHAR(100) NOT NULL,
    publication_year INT,
    CONSTRAINT fk_authors
        FOREIGN KEY(author_id)
        REFERENCES authors(id)
);
)"_zv);

    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id UUID,
    tag varchar(30) NOT NULL,
    CONSTRAINT fk_books
        FOREIGN KEY(book_id)
        REFERENCES books(id)
);
)"_zv);

    // коммитим изменения
    work.commit();
}

}  // namespace postgres

