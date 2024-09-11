#include "book.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace domain {

const BookId &Book::GetId() const noexcept {
    return id_;
}

const AuthorId &Book::GetAuthorId() const noexcept {
    return author_id_;
}

const std::string &Book::GetTitle() const noexcept {
    return title_;
}

int Book::GetPublicationYear() const noexcept {
    return publication_year_;
}

}  // namespace domain
