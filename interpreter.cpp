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
#include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#include <fmt/core.h>
#endif
#if __has_include(<sal.h>)
#include <sal.h>
#else
#define _In_
#define _Inout_
#endif
#include <cstddef>
#include <iostream>
#include "config.hpp"
#include "loxo_fwd.hpp"
#include "tools/execution_context.hpp"

// clang-format off
namespace net::ancillarycat::loxograph {
LOXOGRAPH_INITIALIZATION(trace);
nodiscard_msg(loxo_main) extern
int loxo_main(_In_ int ,
              _In_ char**,
              _Inout_ ExecutionContext &);
// clang-format on

} // namespace net::ancillarycat::loxograph
void alterToolContext(
    net::ancillarycat::loxograph::ExecutionContext &execution_context) {
  static auto debugInputFilePath =
      net::ancillarycat::utils::path{"Z:/loxograph/examples/dynamic.lox"};
  if (execution_context.commands.empty())
    execution_context.commands.emplace_back("tokenize");
  if (execution_context.input_files.empty()) {
    if (exists(debugInputFilePath))
      execution_context.input_files.emplace_back(debugInputFilePath);
    else {
      dbg(critical, "file not found: {}", debugInputFilePath);
      LOXOGRAPH_DEBUG_BREAK
    }
  }
}
// clang-format on
int main(int argc, char **argv, char **envp) {

  auto &tool_context =
      net::ancillarycat::loxograph::ExecutionContext::inspectArgs(argc, argv,
                                                                  envp);

#ifdef LOXOGRAPH_DEBUG_ENABLED
  alterToolContext(tool_context);
#endif

  dbg(info, "Executable name: {}", tool_context.executable_name);
  dbg(info, "Executable path: {}", tool_context.executable_path);
  dbg(info, "Command: {}",
      tool_context.commands.empty() ? "<no command provided>"
                                    : tool_context.commands.front());
  dbg(info, "Execution directory: {}", tool_context.execution_dir);
  dbg(info, "Temp directory: {}", tool_context.tempdir);

  return net::ancillarycat::loxograph::loxo_main(argc, argv, tool_context);
}
