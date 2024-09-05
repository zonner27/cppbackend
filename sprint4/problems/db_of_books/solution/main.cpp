#include <iostream>
#include <pqxx/pqxx>
#include <boost/json.hpp>

namespace json = boost::json;
using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

void create_table_if_not_exists(pqxx::connection &conn) {
    // Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
    // Транзакция нужна, чтобы выполнять запросы.
    pqxx::work transaction(conn);
    // Используя транзакцию создадим таблицу в выбранной базе данных:
    transaction.exec(
        "CREATE TABLE IF NOT EXISTS books (id SERIAL PRIMARY KEY, title varchar(100) NOT NULL, author varchar(100) NOT NULL, year integer NOT NULL, ISBN char(13) UNIQUE);"_zv);
    transaction.exec("DELETE FROM books;"_zv);
    // Применяем все изменения
    transaction.commit();
}

bool add_book(pqxx::connection &conn, const json::object &payload) {
    try {
        pqxx::work txn(conn);
        std::string title = payload.at("title").as_string().c_str();
        std::string author = payload.at("author").as_string().c_str();
        int year = payload.at("year").as_int64();
        std::string isbn = payload.contains("ISBN") && !payload.at("ISBN").is_null() ? payload.at("ISBN").as_string().c_str() : "";

        if (isbn.empty()) {
            txn.exec_params(
                "INSERT INTO books (title, author, year) VALUES ($1, $2, $3)", title, author, year);
        } else {
            txn.exec_params(
                "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4)", title, author, year, isbn);
        }
        txn.commit();
        return true;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

json::array get_all_books(pqxx::connection &conn) {
    pqxx::read_transaction txn(conn);
    pqxx::result res = txn.exec(R"(
        SELECT id, title, author, year, ISBN FROM books
        ORDER BY year DESC, title ASC, author ASC, ISBN ASC NULLS LAST
    )");

    json::array books;
    for (const auto &row : res) {
        json::object book;
        book["id"] = row["id"].as<int>();
        book["title"] = row["title"].as<std::string>();
        book["author"] = row["author"].as<std::string>();
        book["year"] = row["year"].as<int>();
        book["ISBN"] = row["ISBN"].is_null() ? nullptr : json::value(row["ISBN"].as<std::string>());
        books.push_back(book);
    }
    return books;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Usage: connect_db <conn-string>\n"sv;
        return EXIT_SUCCESS;
    } else if (argc != 2) {
        std::cerr << "Invalid command line\n"sv;
        return EXIT_FAILURE;
    }

    try {
        // Подключение к базе данных
        pqxx::connection conn(argv[1]);

        // Создание таблицы, если она не существует
        create_table_if_not_exists(conn);

        std::string input;
        while (std::getline(std::cin, input)) {
            json::value parsed_json = json::parse(input);
            json::object request = parsed_json.as_object();
            std::string action = request.at("action").as_string().c_str();
            json::object payload = request.at("payload").as_object();

            if (action == "add_book") {
                bool success = add_book(conn, payload);
                json::object result = {{"result", success}};
                std::cout << json::serialize(result) << std::endl;
            } else if (action == "all_books") {
                json::array books = get_all_books(conn);
                std::cout << json::serialize(books) << std::endl;
            } else if (action == "exit") {
                break;
            } else {
                std::cerr << "Unknown action: " << action << std::endl;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
