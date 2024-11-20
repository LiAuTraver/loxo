#pragma once

#include <vector>

#include "config.hpp"
#include "file_reader.hpp"
#include "lex_error.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"
#include "Token.hpp"

/// @namespace net::ancillarycat::loxograph
namespace net::ancillarycat::loxograph {
class lexer {
public:
  using size_type = typename utils::string::size_type;
  using string_type = utils::string;
  using string_view_type = utils::string_view;
  using path_type = utils::path;
  using status_t = utils::Status;
  using token_t = Token;
  using token_type_t = token_t::token_type;
  using tokens_t = std::vector<token_t>;
  using lexeme_views_t = std::vector<string_view_type>;
  using file_reader_t = utils::file_reader;
  using char_t = typename string_type::value_type;
  using error_t = lex_error;
  using error_code_t = typename error_t::type_t;

public:
  explicit lexer() = default;

  lexer(const lexer &other) = delete;
  lexer(lexer &&other) = delete;

  lexer &operator=(const lexer &other) = delete;
  lexer &operator=(lexer &&other) = delete;

  ~lexer() = default;

public:
public:
  /// @brief load the contents of the file
  /// @return OkStatus() if successful, NotFoundError() otherwise
  status_t load(const path_type &);
  /// @copydoc load(const path_type &)
  status_t load(string_type &&);
  /// @copydoc load(const path_type &)
  status_t load(const std::istream &);
  /// @brief lex the contents of the file
  /// @return OkStatus() if successful, NotFoundError() otherwise
  status_t lex();
  auto get_tokens() -> tokens_t;
  bool ok() const noexcept;
  uint_least32_t error() const noexcept;

private:
  void add_identifier();
  void add_number();
  void add_string();
  void add_comment();
  void next_token();
  void add_token(token_type_t, std::any = std::any());
  void add_lex_error(lex_error::type_t= error_t::kMonostate);
  bool is_at_end(size_t = 0) const;
  auto lex_string() -> lexer::status_t::Code;
  auto lex_identifier() -> string_view_type;
  auto lex_number(bool) -> std::any;

private:
  /// @brief lookaheads; we have only consumed the character before the cursor
  char_t peek(size_t offset = 0);
  /// @brief get current character and advance the cursor
  /// @note does not check if @code cursor + offset >= contents.size() @endcode
  const char_t &get(size_t offset = 1);

  /// @brief advance the cursor if the character is the expected character
  /// @param expected the expected character
  /// @return true if the character is the expected character and the cursor is
  /// advanced, false otherwise
  bool advance_if_is(char_t expected);

  /// @brief advance the cursor if the predicate is true
  /// @tparam Predicate the predicate to check
  /// @param predicate the predicate to check
  /// @return true if the predicate is true and the cursor is advanced, false
  template <typename Predicate>
  bool advance_if(Predicate &&predicate)
    requires std::invocable<Predicate, char_t> &&
             std::convertible_to<Predicate, bool>;

public:
  nodiscard_msg(token_views_t) const tokens_t &get_tokens() const {
    return tokens;
  }

private:
  /// @brief convert a string to a number
  /// @tparam Num the number type
  /// @param value the string to convert
  /// @return the number if successful, std::any() otherwise
  template <typename Num>
    requires std::is_arithmetic_v<Num>
  std::any to_number(string_view_type value);

private:
  /// @brief head of a token
  size_type head = 0;
  /// @brief current cursor position
  size_type cursor = 0;
  /// @brief the contents of the file
  const string_type contents = string_type();
  /// @brief lexme views(non-owning)
  lexeme_views_t lexeme_views = lexeme_views_t();
  /// @brief current source line number
  uint_least32_t current_line = 1;
  /// @brief tokens
  tokens_t tokens = tokens_t();
  /// @brief errors
  uint_least32_t error_count = 0;
};
} // namespace net::ancillarycat::loxograph