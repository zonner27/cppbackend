#pragma once
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "../domain/tag_fwd.h"
#include "use_cases.h"


namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors,
                          domain::BookRepository& books, domain::TagRepository& tags)
        : authors_{authors}
        , books_{books}
        , tags_{tags} {
    }    

    void AddAuthor(const std::string& name) override;
    void DeleteAuthor(const std::string& id) override;
    void EditAuthor(const std::string& id, const std::string& new_name) override;
    std::vector<ui::detail::AuthorInfo> GetAllAuthors() override;
    ui::detail::BookInfo AddBook(const std::string& author_id, const std::string& title, int year) override;
    void DeleteBook(const std::string& id) override;
    std::vector<ui::detail::BookInfo> GetBooksBy(const std::string& author_id) override;
    ui::detail::BookInfoAllBooks GetBookByBookId(const std::string &id) override;
    std::vector<ui::detail::BookInfo> GetAllbooks() override;
    std::vector<ui::detail::BookInfoAllBooks> GetAllBooksForShow() override;
    std::vector<ui::detail::BookInfoAllBooks> GetBooksByTitle(const std::string title) override;
    void AddTagsBooks(std::set<std::string>, const std::string& book_id) override;
    std::optional<std::string> GetTagsByBookId(const std::string &book_id) override;
    void UpdateBook(const std::string &id,
                const std::string& new_title, int new_year) override;
    void DeleteTagsByBookId(const std::string book_id) override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
    domain::TagRepository& tags_;
};

}  // namespace app
