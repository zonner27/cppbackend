#pragma once
#include <string>
#include <vector>
#include <optional>

#include "../util/tagged_uuid.h"
#include "book.h"

namespace domain {

namespace detail {
struct TagTag {};
}  // namespace detail

using TagId = util::TaggedUUID<detail::TagTag>;

class Tag {
public:
    Tag(BookId book_id, std::string tag_str)
        : book_id_(std::move(book_id))
        , tag_str_(std::move(tag_str))
    {}

    const BookId& GetBookId() const noexcept;
    const std::string& GetTag() const noexcept;

private:
    BookId book_id_;
    std::string tag_str_;
};

class TagRepository {
public:
    virtual void Save(const Tag& tag) = 0;
    virtual std::optional<std::string> GetTagsByBookId(const std::string book_id) = 0;
    virtual void DeleteTagsByBookId(const std::string book_id) = 0;


protected:
    ~TagRepository() = default;
};

}  // namespace domain
