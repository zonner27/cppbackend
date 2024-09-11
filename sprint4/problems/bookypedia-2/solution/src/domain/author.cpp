#include "author.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace domain {

const AuthorId &Author::GetId() const noexcept {
    return id_;
}

const std::string &Author::GetName() const noexcept {
    return name_;
}

}  // namespace domain
