#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <set>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    out << book.title << ", " << book.publication_year;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfoAllBooks& book) {
    out << book.title << " by " << book.author_name<< ", " << book.publication_year;
    return out;
}

}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}


std::set<std::string> normalizeTags(const std::string& input) {

    std::set<std::string> tags;
    std::string trimmed_input = boost::algorithm::trim_copy(input);  // ”дал€ем пробелы по кра€м
    std::istringstream ss(trimmed_input);
    std::string tag;
    while (std::getline(ss, tag, ',')) {
        boost::algorithm::trim(tag);
        // ”дал€ем лишние пробелы между словами
        boost::algorithm::replace_all(tag, "  ", " ");  // «амен€ем двойные пробелы на один пробел
        while (tag.find("  ") != std::string::npos) {  // ѕродолжаем до удалени€ всех двойных пробелов
            boost::algorithm::replace_all(tag, "  ", " ");
        }
        if (!tag.empty()) {
            tags.insert(tag);
        }
    }
    return tags;
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, //std::bind(&View::AddAuthor, this, ph::_1)
        // либо
         [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );

    menu_.AddAction(
        "DeleteAuthor"s, "name"s, "Dels author"s,
         [this](auto& cmd_input) { return DeleteAuthor(cmd_input); });

    menu_.AddAction("EditAuthor"s, "name"s, "Editss author"s,
         [this](auto& cmd_input) { return EditAuthor(cmd_input); });

    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));

    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));

    menu_.AddAction("DeleteBook"s, "name"s, "Dels booj"s,
         [this](auto& cmd_input) { return DeleteBook(cmd_input); });

    menu_.AddAction("EditBook"s, "name"s, "Editss book"s,
         [this](auto& cmd_input) { return EditBook(cmd_input); });

    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowBook"s, "name"s, "Show book"s,
         [this](auto& cmd_input) { return ShowBook(cmd_input); });

    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
                    std::bind(&View::ShowAuthorBooks, this));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if(name.empty()) {
            throw std::runtime_error("New name is empty");
        }
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream &cmd_input) const  {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        std::optional<std::string> author_id;

        if(name.empty()) {
            auto authors = GetAuthors();
            if (authors.empty()) {
                return true;
            }
            author_id = SelectAuthor(authors);
        } else {
            author_id = GetAuthorsIdByAuthorName(name);
        }
        if (!author_id.has_value()) {
            throw std::runtime_error("New name is empty");
        }
        if (author_id == "exit") {
            return true;
        }
        use_cases_.DeleteAuthor(author_id.value());

    } catch (const std::exception&) {
        output_ << "Failed to delete author"sv << std::endl;
    }
    return true;
}

bool View::EditAuthor(std::istream &cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        std::optional<std::string> author_id;

        if(name.empty()) {
            auto authors = GetAuthors();
            if (authors.empty()) {
                return true;
            }
            std::cout << "Select author: " << std::endl;
            author_id = SelectAuthor(authors);
        } else {
            author_id = GetAuthorsIdByAuthorName(name);
        }

        if (not author_id.has_value()) {
            throw std::runtime_error("Author not found");
        }
        if (author_id == "exit") {
            return true;
        }
        std::string new_name;
        std::cout << "Enter new name:" << std::endl;
        std::getline(std::cin, new_name);
        if (new_name.empty()) {
            throw std::runtime_error("New name is empty");
        }
        use_cases_.EditAuthor(author_id.value(), new_name);


    } catch (const std::exception&) {
        output_ << "Failed to edit author"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            if (!params.has_value()) {
                throw std::runtime_error("Parameetres is empty");
            }
            std::cout << "Enter tags (comma separated):" << std::endl;
            std::string input_tags;
            std::getline(std::cin, input_tags);

            std::set<std::string> normalized_tags = normalizeTags(input_tags);

            auto new_book = use_cases_.AddBook(params->author_id, params->title, params->publication_year);
            use_cases_.AddTagsBooks(normalized_tags, new_book.id);
        }
    } catch (const std::exception&) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::DeleteBook(std::istream &cmd_input) const {
    try {
        std::string title;
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);

        std::optional<std::string> book_id;

        if(title.empty()) {
            book_id = SelectBook(); 
        } else {
            auto books = use_cases_.GetBooksByTitle(title);
            if (books.size() == 0) {
                std::cout << "Book not found" << std::endl;
                //throw std::runtime_error("Name is empty");
            } else if (books.size() == 1) {
                book_id = books[0].id;
            } else {
                book_id = SelectBookFromBooks(books);
            }
        }
        if (book_id ==  "exit") {
            return true;
        }
        if (not book_id.has_value()) {
            throw std::runtime_error("Name is empty");
        }

        if (book_id == "exit") {
            return true;
        }
        use_cases_.DeleteBook(book_id.value());


    } catch (const std::exception&) {
        output_ << "Failed to delete book"sv << std::endl;
    }
    return true;
}

bool View::EditBook(std::istream &cmd_input) const {
    try {
        std::string title;
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);
        std::optional<std::string> book_id;

        if(title.empty()) {
            book_id = SelectBook();
        } else {
            auto books = use_cases_.GetBooksByTitle(title);
            if (books.size() == 0) {
                throw std::runtime_error("Name is empty");
            } else if (books.size() == 1) {
                book_id = books[0].id;
            } else {
                book_id = SelectBookFromBooks(books);
            }
        }
        if (not book_id.has_value()) {
            throw std::runtime_error("Name is empty");
        }
        auto book = GetBookByBookId(book_id.value());
        std::string new_title;
        std::cout << "Enter new title or empty line to use the current one"
                  << "(" << book.title << "):" << std::endl;
        std::getline(std::cin, new_title);
        if (new_title.empty()) {
            new_title = book.title;
        }

        std::string year_input;
        int new_year;
        std::cout << "Enter publication year or empty line to use the current one "
                  << "(" << book.publication_year << "):" << std::endl;
        std::getline(std::cin, year_input);
        if (year_input.empty()) {
            new_year = book.publication_year;
        } else {
            new_year = std::stoi(year_input);
        }

        std::optional<std::string> tags = use_cases_.GetTagsByBookId(book.id);
        std::string new_tags;

        if (tags.has_value()) {
            std::cout << "Enter tags "
                  << "(" << tags.value() << "):" << std::endl;
        } else {
            std::cout << "Enter tags:" << std::endl;
        }
        std::getline(std::cin, new_tags);

        use_cases_.UpdateBook(book_id.value(), new_title, new_year);

        if (tags->empty()) {
             std::set<std::string> normalized_tags = normalizeTags(new_tags);
             use_cases_.AddTagsBooks(normalized_tags, book_id.value());
        } else {
            use_cases_.DeleteTagsByBookId(book_id.value());
            std::set<std::string> normalized_tags = normalizeTags(new_tags);
            use_cases_.AddTagsBooks(normalized_tags, book_id.value());
        }


    } catch (const std::exception&) {
        output_ << "Book not found"sv << std::endl;
    }
    return true;
}

bool View::AddTags(std::set<std::string>& tags, std::string &book_id) const {
    try {

    } catch (const std::exception&) {
        output_ << "Failed to add book_tags"sv << std::endl;
    }
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetAllBooksForShow());
    return true;
}

void View::printBook(const ui::detail::BookInfoAllBooks& book) const{

   std::optional<std::string> tags = use_cases_.GetTagsByBookId(book.id);

    std::cout << "Title: " << book.title << std::endl
              << "Author: " << book.author_name << std::endl
              << "Publication year: " << book.publication_year << std::endl;
    if (tags) {
              std::cout << "Tags: " << tags.value() << std::endl;
    }
}

bool View::ShowBook(std::istream& cmd_input) const{

    try {
        std::string title;
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);

        std::vector<ui::detail::BookInfoAllBooks> books;

        if (title.empty()) {
             books = use_cases_.GetAllBooksForShow();
        } else {
            books = use_cases_.GetBooksByTitle(title);
        }

        if (books.size() == 0) {
            return true;
        } else if (books.size() == 1 && !title.empty()) {
            printBook(books[0]);
        } else {
            PrintVector(output_, books);
            std::string answer;
            std::cout << "Enter the book # or empty line to cancel:" << std::endl;
            std::getline(std::cin, answer);
            boost::algorithm::trim(title);

            if (!answer.empty() && std::all_of(answer.begin(), answer.end(), ::isdigit)) {
                int selected_index = std::stoi(answer);
                if (selected_index >= 1 && selected_index <= books.size()) {
                    printBook(books[selected_index - 1]);
                } else {
                    return true;
                }
            } else {
                return true;
            }
        }
    } catch (const std::exception&) {
        output_ << "Failed to show book"sv << std::endl;
    }

    return true;
}

bool View::ShowAuthorBooks() const {
    try {
        std::cout << "Select author: " << std::endl;
        auto authors = GetAuthors();
        if (authors.empty()) {
            return true;
        }
        if (auto author_id = SelectAuthor(authors)) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}


std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);

    std::cout << "Enter author name or empty line to select from list:" << std::endl;
    std::string author_name;
    std::getline(std::cin, author_name);
    if (author_name.empty()) {
        std::cout << "Select author: " << std::endl;
        auto authors = GetAuthors();
        if (authors.empty()) {
            return std::nullopt;
        }
        auto author_id = SelectAuthor(authors);
        if (not author_id.has_value())
            return std::nullopt;
        else {
            params.author_id = author_id.value();
            return params;
        }
    } else {
        auto authors = GetAuthors();

        auto it = std::find_if(authors.begin(), authors.end(),
                [&author_name](const ui::detail::AuthorInfo& author) {
                    return author.name == author_name;
        });

        if (it != authors.end()) {
            params.author_id = it->id;
            return params;
        } else {
            std::string answer;
            std::cout << "No author found. Do you want to add " << author_name << " (y/n)?" << std::endl;
            std::getline(std::cin, answer);
            if (answer == "y" || answer == "Y") {
                std::istringstream name_input(author_name);
                AddAuthor(name_input);
                authors = GetAuthors();
                auto it_name = std::find_if(authors.begin(), authors.end(),
                        [&author_name](const ui::detail::AuthorInfo& author) {
                            return author.name == author_name;
                });
                if (it != authors.end()) {
                    params.author_id = it_name->id;
                    return params;
                } else {
                    throw std::runtime_error("Failed add authors");
                }
            } else {
                throw std::runtime_error("Failed answer");;
            }
        }

    }

    return params;
}

std::optional<std::string> View::SelectAuthor(std::vector<detail::AuthorInfo>& authors) const {

    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str)) {
        return std::nullopt;
    }
    if (str.empty()) {
        return "exit";
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

std::optional<std::string> View::SelectBook() const {
    auto books = use_cases_.GetAllBooksForShow();
    if (books.size() == 0) {
       return "exit";
    }
    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str)) {
        return std::nullopt;
    }

    if (str.empty()) {
        return "exit";
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return books[book_idx].id;

}

std::optional<std::string> View::SelectBookFromBooks(std::vector<ui::detail::BookInfoAllBooks> books) const {
    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str)) {
        return std::nullopt;
    }
    if (str.empty()) {
        return "exit";
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return books[book_idx].id;

}


std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_autors;
//    assert(!"TODO: implement GetAuthors()");
    try {
        dst_autors = use_cases_.GetAllAuthors();
    } catch (const std::exception&) {
        output_ << "Failed to show authors"sv << std::endl;
    }
    return dst_autors;
}

std::optional<std::string> View::GetAuthorsIdByAuthorName(const std::string &author_name) const {
    std::string author_id;
    auto authors = GetAuthors();
    auto it = std::find_if(authors.begin(), authors.end(),
            [&author_name](const ui::detail::AuthorInfo& author) {
                return author.name == author_name;
    });
    if (it != authors.end()) {
        author_id = it->id;
    } else {
        return std::nullopt;
    }
    return author_id;
}

detail::BookInfoAllBooks View::GetBookByBookId(const std::string &id) const{
    return use_cases_.GetBookByBookId(id);
}

std::vector<detail::BookInfo> View::GetBooks() const {
    std::vector<detail::BookInfo> books;
    books = use_cases_.GetAllbooks();
    return books;
}

std::vector<detail::BookInfoAllBooks> View::GetAllBooksForShow() const {
    std::vector<detail::BookInfoAllBooks> books;
    books = use_cases_.GetAllBooksForShow();
    return books;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
    std::vector<detail::BookInfo> books;
    books = use_cases_.GetBooksBy(author_id);
    //assert(!"TODO: implement GetAuthorBooks()");
    return books;
}

}  // namespace ui
