#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#if __has_include(<spdlog/spdlog.h>)
#  include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#  include <fmt/core.h>
#  include <fmt/format.h>
#endif
#include "execution_context.hpp"

AC_SPDLOG_INITIALIZATION(loxo, info);

namespace accat = net::ancillarycat;

void alterToolContext(accat::loxo::ExecutionContext &execution_context) {
  static auto debugInputFilePath =
      std::filesystem::path{"Z:/loxo/examples/dynamic.lox"};
  if (execution_context.commands.empty())
    execution_context.commands.emplace_back(
        accat::loxo::ExecutionContext::interpret);
  if (execution_context.input_files.empty()) {
    if (exists(debugInputFilePath))
      execution_context.input_files.emplace_back(debugInputFilePath);
    else {
      dbg(critical,
          "file not found: {}."
          "you are in debug mode, please provide a file.",
          debugInputFilePath)
      AC_UTILS_DEBUG_BREAK
    }
  }
}
// NOLINTNEXTLINE // <-- why clang-tidy warns the main function?
int main(int argc, char **argv, char **envp) {

  auto &tool_context =
      accat::loxo::ExecutionContext::inspectArgs(argc, argv, envp);

  dbg_block(alterToolContext(tool_context);

            dbg(info, "Executable name: {}", tool_context.executable_name);
            dbg(info, "Executable path: {}", tool_context.executable_path);
            dbg(info,
                "Command: {}",
                tool_context.commands.empty()
                    ? "<no command provided>"
                    : accat::loxo::ExecutionContext::command_sv(
                          tool_context.commands.front()));
            // dbg(info, "Input files: {}", tool_context.input_files);
            // MSVC failed                    ^^^^^^^^(a vec of path)
            std::ranges::for_each(
                tool_context.input_files,
                [](const auto &file) { dbg(info, "Input file: {}", file); });
            dbg(info, "Execution directory: {}", tool_context.execution_dir);
            dbg(info, "Temp directory: {}", tool_context.tempdir))

  return accat::loxo::loxo_main(argc, argv, tool_context);
}
