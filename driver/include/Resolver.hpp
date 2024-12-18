#pragma once

#include "details/loxo_fwd.hpp"

#include "details/IVisitor.hpp"
#include "ExprVisitor.hpp"
#include "StmtVisitor.hpp"

namespace net::ancillarycat::loxo {
// TODO: Implement the Resolver class
class Resolver : virtual public expression::ExprVisitor,
                 virtual public statement::StmtVisitor,
                 public std::enable_shared_from_this<Resolver> {
public:
  inline explicit Resolver(interpreter &interpreter);
  virtual ~Resolver() override = default;

private:
  interpreter &interpreter;

private:
  auto visit_impl(const expression::Literal &) const -> eval_result_t override;
  auto visit_impl(const expression::Unary &) const -> eval_result_t override;
  auto visit_impl(const expression::Binary &) const -> eval_result_t override;
  auto visit_impl(const expression::Grouping &) const -> eval_result_t override;
  auto visit_impl(const expression::Variable &) const -> eval_result_t override;
  auto visit_impl(const expression::Assignment &) const
      -> eval_result_t override;
  auto visit_impl(const expression::Logical &) const -> eval_result_t override;
  auto visit_impl(const expression::Call &) const -> eval_result_t override;
  auto evaluate_impl(const expression::Expr &) const -> eval_result_t override;
  auto get_result_impl() const -> eval_result_t override;

private:
  auto visit_impl(const statement::Variable &) const -> eval_result_t override;
  auto visit_impl(const statement::Print &) const -> eval_result_t override;
  auto visit_impl(const statement::Expression &) const
      -> eval_result_t override;
  auto visit_impl(const statement::Block &) const -> eval_result_t override;
  auto visit_impl(const statement::If &) const -> eval_result_t override;
  auto visit_impl(const statement::While &) const -> eval_result_t override;
  auto visit_impl(const statement::For &) const -> eval_result_t override;
  auto visit_impl(const statement::Function &) const -> eval_result_t override;
  auto visit_impl(const statement::Return &) const -> eval_result_t override;
  auto execute_impl(const statement::Stmt &) const -> eval_result_t override;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};

} // namespace net::ancillarycat::loxo
