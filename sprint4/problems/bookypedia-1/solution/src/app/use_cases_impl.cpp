#include "use_cases_impl.h"
#include "../domain/author.h"
#include "../domain/book.h"

#include <boost/uuid/uuid_io.hpp>
#include <string>
#include <sstream>


namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

std::vector<ui::detail::AuthorInfo> UseCasesImpl::GetAllAuthors(){

    std::vector<ui::detail::AuthorInfo> authors_info;
    for (auto& author : authors_.GetAllAuthors()) {
        std::string id = boost::uuids::to_string(*author.GetId());
        authors_info.emplace_back(id, author.GetName());
    }
    return authors_info;
}

void UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, int year) {
    books_.Save({BookId::New(), AuthorId::FromString(author_id), title, year});
}


std::vector<ui::detail::BookInfo> UseCasesImpl::GetBooksBy(const std::string& author_id) {
    std::vector<ui::detail::BookInfo> list_books;
    for (const auto& book : books_.GetBooksBy(author_id)) {
        list_books.emplace_back(book.GetTitle(), book.GetPublicationYear());
    }
    return list_books;
}

std::vector<ui::detail::BookInfo> UseCasesImpl::GetAllbooks() {
    std::vector<ui::detail::BookInfo> list_books;
    for (const auto& book : books_.GetAllBooks()) {
        list_books.emplace_back(book.GetTitle(), book.GetPublicationYear());
    }
    return list_books;
}

}  // namespace app
