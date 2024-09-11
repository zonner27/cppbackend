#pragma once
#include <string>
#include <vector>
#include <optional>

#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept;
    const std::string& GetName() const noexcept;

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    virtual void Save(const Author& author) = 0;
    virtual void Delete(const std::string& id) = 0;
    virtual void UpdateName(const std::string& id, const std::string& new_name) = 0;
    virtual std::vector<Author> GetAllAuthors() = 0;

protected:
    ~AuthorRepository() = default;
};

}  // namespace domain
