#include "use_cases_impl.h"
#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"

#include <boost/uuid/uuid_io.hpp>
#include <string>
#include <sstream>
#include <iostream>


namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

void UseCasesImpl::DeleteAuthor(const std::string &id) {
    authors_.Delete(id);
}

void UseCasesImpl::EditAuthor(const std::string &id, const std::string &new_name) {
    authors_.UpdateName(id, new_name);
}

std::vector<ui::detail::AuthorInfo> UseCasesImpl::GetAllAuthors(){

    std::vector<ui::detail::AuthorInfo> authors_info;
    for (auto& author : authors_.GetAllAuthors()) {
        std::string id = boost::uuids::to_string(*author.GetId());
        authors_info.emplace_back(id, author.GetName());
    }
    return authors_info;
}

ui::detail::BookInfo UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, int year) {
    BookId id = BookId::New();
    ui::detail::BookInfo book{id.ToString(), title, year};
    books_.Save({id, AuthorId::FromString(author_id), title, year});
    return book;
}

void UseCasesImpl::DeleteBook(const std::string &id){
    books_.Delete(id);
}

std::vector<ui::detail::BookInfo> UseCasesImpl::GetBooksBy(const std::string& author_id) {
    std::vector<ui::detail::BookInfo> list_books;
    for (const auto& book : books_.GetBooksBy(author_id)) {
        list_books.emplace_back(book.GetId().ToString(), book.GetTitle(), book.GetPublicationYear());
    }
    return list_books;
}

ui::detail::BookInfoAllBooks UseCasesImpl::GetBookByBookId(const std::string &id){
    return books_.GetBook(id);
}

std::vector<ui::detail::BookInfo> UseCasesImpl::GetAllbooks() {
    std::vector<ui::detail::BookInfo> list_books;
    for (const auto& book : books_.GetAllBooks()) {
        list_books.emplace_back(book.GetId().ToString(), book.GetTitle(), book.GetPublicationYear());
    }
    return list_books;
}

std::vector<ui::detail::BookInfoAllBooks> UseCasesImpl::GetAllBooksForShow() {
    return books_.GetAllBooksForShow();
}

std::vector<ui::detail::BookInfoAllBooks> UseCasesImpl::GetBooksByTitle(const std::string title){
    return books_.GetBooksBytitle(title);
}

void UseCasesImpl::AddTagsBooks(std::set<std::string> tags_set, const std::string& book_id) {
    for (auto& tag : tags_set) {
        tags_.Save({BookId::FromString(book_id), tag});
    }
}

std::optional<std::string> UseCasesImpl::GetTagsByBookId(const std::string &book_id){
    return tags_.GetTagsByBookId(book_id);
}

void UseCasesImpl::UpdateBook(const std::string &id, const std::string& new_title, int new_year){
    books_.UpdateBook(id, new_title, new_year);
}

void UseCasesImpl::DeleteTagsByBookId(const std::string book_id){
    tags_.DeleteTagsByBookId(book_id);
}

}  // namespace app
