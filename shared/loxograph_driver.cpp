#include <algorithm>
#include <cstddef>
#include <print>
#include <ranges>
#if __has_include(<spdlog/spdlog.h>)
#  include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#  include <fmt/core.h>
#endif
#if __has_include(<sal.h>)
#  include <sal.h>
#else
#  define _In_
#  define _Inout_
#  define _In_opt_
#endif
#ifdef _WIN32
#  include <io.h>
#  define isatty _isatty
#  define fileno _fileno
#else
#  include <unistd.h>
#endif

#include "config.hpp"
#include "execution_context.hpp"
#include "ExprVisitor.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph {
utils::Status show_msg() {
  dbg(critical, "please provide a command.");
  contract_assert(false);
  return utils::Status::kEmptyInput;
}
utils::Status onFileOperationFailed(const utils::Status &load_result) {
  dbg(error, "{}", load_result.message().data());
  return utils::Status::kPermissionDeniedError;
}
utils::Status onLexOperationFailed(const utils::Status &lex_result) {
  dbg(warn, "{}", lex_result.message().data());
  return utils::Status::kError;
}
utils::Status onLexOperationExit(const ExecutionContext &ctx) {
  dbg(error, "lexing process completed with {} error(s).", ctx.lexer->error());
  return utils::Status::kError;
}
utils::Status onCommandNotFound(const ExecutionContext &ctx) {
  dbg(error,
      "Unknown command: {}",
      ExecutionContext::command_sv(ctx.commands.front()));
  return utils::Status::kCommandNotFound;
}
void writeLexResultsToContextStream(ExecutionContext &ctx,
                                    const lexer::tokens_t &tokens) {
  std::ranges::for_each(tokens, [&ctx](const auto &token) {
    if (token.type.type == TokenType::kLexError) {
      ctx.error_stream << token.to_string() << std::endl;
    }
  });
  std::ranges::for_each(tokens, [&ctx](const auto &token) {
    if (token.type.type != TokenType::kLexError) {
      ctx.output_stream << token.to_string() << std::endl;
    }
  });
}
utils::Status tokenize(ExecutionContext &ctx) {
  if (ctx.input_files.size() != 1) {
    return show_msg();
  }
  ctx.lexer = std::make_shared<class lexer>();
  if (const utils::Status load_result =
          ctx.lexer->load(*ctx.input_files.cbegin());
      !load_result.ok()) {
    return onFileOperationFailed(load_result);
  }
  const utils::Status lex_result = ctx.lexer->lex();
  if ( !lex_result.ok()) {
    return onLexOperationFailed(lex_result);
  }
  const auto tokens = ctx.lexer->get_tokens();
  if (!ctx.lexer->ok()) {
    return onLexOperationExit(ctx);
  }
  dbg(info, "lexing process completed successfully with no errors.");
  return utils::Status::kOkStatus;
}
utils::Status parse(ExecutionContext &ctx) {
  dbg(info, "Parsing...");
  ctx.parser = std::make_shared<class parser>();
  ctx.parser->set_views(ctx.lexer->get_tokens());
  auto res = ctx.parser->parse();
  return res;
}
utils::Status interpret(ExecutionContext &ctx) {
  dbg(info, "evaluating...");
  ctx.interpreter = std::make_shared<expression::ExprEvaluator>();
  auto _ = ctx.interpreter->evaluate(*ctx.parser->get_expr());
  dbg(info, "evaluation completed.");
  return _.code();
}
void writeParseResultToContextStream(ExecutionContext &ctx) {
  expression::ASTPrinter astPrinter;
  auto _ = ctx.parser->get_expr()->accept(astPrinter);
  ctx.output_stream << astPrinter.to_string();
}
void writeExprResultToContextStream(ExecutionContext &ctx) {
  // add missing newline character
  ctx.output_stream << ctx.interpreter->to_string() << std::endl;
}
// clang-format off
nodiscard_msg(loxo_main)
int loxo_main(_In_ const int argc,
              _In_opt_ char **argv, //! @note argv can be nullptr(debug mode or google test)
              _Inout_ ExecutionContext &ctx)
// clang-format on
{
  if (!argv) {
    dbg(info, "Debug mode enabled.");
  }
  if (argc == 0) {
    dbg(critical, "No arguments provided.");
    return 1;
  }
  if (ctx.commands.empty()) {
    std::println(stderr, "No command provided.");
    return 1;
  }
  if (ctx.input_files.empty()) {
    std::println(stderr, "No input files provided.");
    return 1;
  }
  utils::Status lex_result;
  if (ctx.commands.front() & ExecutionContext::needs_lex) {
    lex_result = tokenize(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::lex) {
    auto tokens = ctx.lexer->get_tokens();
    writeLexResultsToContextStream(ctx, tokens);
    // codecrafter's test needs stdout and stderr
    std::cerr << ctx.error_stream.str();
    std::cout << ctx.output_stream.str() << std::endl;
    return lex_result.ok() ? 0 : 65;
  }
  utils::Status parse_result;
  if (ctx.commands.front() & ExecutionContext::needs_parse) {
    parse_result = parse(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::parse) {
    if (parse_result.ok()) {
      writeParseResultToContextStream(ctx);
      std::cout << ctx.output_stream.str() << std::endl;
      return 0;
    } else {
      dbg(error, "Parsing failed: {}", parse_result.message());
      ctx.error_stream << parse_result.message() << std::endl;
      // for codecrafter's test
      std::cerr << parse_result.message() << std::endl;
      return 65;
    }
  }
  utils::Status interpret_result;
  if (ctx.commands.front() & ExecutionContext::needs_interpret) {
    interpret_result = interpret(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::interpret) {
    if (interpret_result.ok()) {
      writeExprResultToContextStream(ctx);
      std::cout << ctx.output_stream.str() << std::endl;
      return 0;
    } else {
      dbg(error, "Interpretation failed: {}", interpret_result.message());
      ctx.error_stream << interpret_result.message() << std::endl;
      return 65;
    }
  }
  return onCommandNotFound(ctx).code();
}
} // namespace net::ancillarycat::loxograph