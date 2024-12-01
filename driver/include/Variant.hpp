#pragma once

#include <utility>
#include <variant>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "fmt.hpp"

namespace net::ancillarycat::utils {
template <typename... Types> class Variant : public Printable, Viewable {
public:
  using variant_type = std::variant<Types...>;

public:
  inline constexpr Variant() = default;

  template <typename Ty>
    requires(!std::same_as<std::decay_t<Ty>, Variant>)
  Variant(Ty &&value) : my_variant(std::forward<Ty>(value)) {}

  Variant(const Variant &) = default;
  Variant(Variant &&) noexcept = default;
  Variant &operator=(const Variant &) = default;
  Variant &operator=(Variant &&) noexcept = default;
  virtual ~Variant() override = default;

public:
  template <typename Callable>
  auto visit(this auto &&self, Callable &&callable) -> decltype(auto) {
  using ReturnType = decltype(std::forward<Callable>(callable)(
      std::declval<variant_type>()));
    return self.is_valid()
               ? std::visit(std::forward<Callable>(callable), self.my_variant)
               : ReturnType{};
  }
  string_view_type type_name() const {
    return is_valid() ? this->visit([]([[maybe_unused]] const auto &value)
                                        -> string_view_type {
      return typeid(value).name();
    })
                      : "invalid state"sv;
  }
  auto index() const noexcept { return my_variant.index(); }
  template <typename... Args>
    requires requires {
      std::declval<variant_type>().template emplace<Args...>(
          std::declval<Args>()...);
    }
  auto emplace(Args &&...args) -> decltype(auto) {
    return my_variant.template emplace<Args...>(std::forward<Args>(args)...);
  }
  template <typename Args>
    requires requires { std::declval<variant_type>().template emplace<Args>(); }
  constexpr auto emplace() -> decltype(auto) {
    return my_variant.template emplace<Args>();
  }
  constexpr auto &get() const { return my_variant; }
  constexpr auto swap(Variant &that) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<Types...>,
                         std::is_nothrow_swappable<Types...>>) -> Variant & {
    my_variant.swap(that.my_variant);
    return *this;
  }
  string_type underlying_string() const {
    return this->visit([](const auto &value) -> string_type {
      return value.to_string(FormatPolicy::kDefault);
    });
  }

private:
  variant_type my_variant{Monostate{}};

private:
  inline constexpr bool is_valid() const noexcept {
    auto ans = my_variant.index() != std::variant_npos;
    contract_assert(ans);
    return ans;
  }

private:
  constexpr auto to_string_impl(const FormatPolicy &format_policy) const
      -> string_type override {
    return typeid(decltype(*this)).name();
  }
  constexpr auto to_string_view_impl(const FormatPolicy &format_policy) const
      -> string_view_type override {
    return typeid(decltype(*this)).name();
  }
};
template <class Ty, class... Types>
inline constexpr bool holds_alternative(const Variant<Types...> &v) noexcept {
  return std::holds_alternative<Ty>(v.get());
}
template <class Ty, class... Types>
inline constexpr auto get(const Variant<Types...> &v) -> decltype(auto) {
  return std::get<Ty>(v.get());
}
template <class Ty, class... Types>
inline constexpr auto get_if(Variant<Types...> *v) noexcept {
  return std::get_if<Ty>(&v->get());
}
} // namespace net::ancillarycat::utils
