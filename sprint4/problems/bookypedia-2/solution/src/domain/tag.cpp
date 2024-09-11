#include "tag.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace domain {

const BookId &Tag::GetBookId() const noexcept {
    return book_id_;
}

const std::string &Tag::GetTag() const noexcept {
    return tag_str_;
}

}  // namespace domain
