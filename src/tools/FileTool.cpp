#include "FileReadTool.h"

#include <fstream>
#include <sstream>

namespace oop_agent {

std::string_view FileReadTool::get_name() const noexcept {
    return "file_read";
}

std::string_view FileReadTool::get_description() const noexcept {
    return "Read a file";
}

std::expected<std::string, ToolError>
FileReadTool::execute(const std::string &arguments) {
    std::ifstream file(arguments);

    if (!file) {
        return std::unexpected(ToolError::NotFound);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

std::expected<std::string, ToolError>
FileReadTool::execute(const std::string &arguments) {
  std::ifstream file(arguments);
  if (!file) {
    return std::unexpected(ToolError::NotFound);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::expected<std::string, ToolError>
FileWriteTool::execute(const std::string &arguments) {
  std::stringstream ss(arguments);
  std::string filename;
  ss >> filename;

  std::string content;
  std::getline(ss, content);
  if (!content.empty() && content[0] == ' ') {
    content = content.substr(1);
  }

  std::ofstream file(filename);
  if (!file) {
    return std::unexpected(ToolError::ExecutionFailed);
  }

  file << content;
  return "OK";
}

} // namespace oop_agent
