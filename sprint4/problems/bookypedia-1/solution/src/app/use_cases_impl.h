#pragma once
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BookRepository& books)
        : authors_{authors}
        , books_{books} {
    }

    void AddAuthor(const std::string& name) override;
    std::vector<ui::detail::AuthorInfo> GetAllAuthors() override;
    void AddBook(const std::string& author_id, const std::string& title, int year) override;
    std::vector<ui::detail::BookInfo> GetBooksBy(const std::string& author_id) override;
    std::vector<ui::detail::BookInfo> GetAllbooks() override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
