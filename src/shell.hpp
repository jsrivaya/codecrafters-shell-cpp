#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

#include <cstdlib>
#include <unistd.h>

namespace shell{

bool is_builtin(const std::string& command) {
    return command.starts_with("exit") ||
           command.starts_with("echo") || 
           command.starts_with("type");
}

bool is_exec(const std::string& path) {
  if(std::filesystem::exists(path)) return (access(path.c_str(), X_OK) == 0);
  return false;
}

std::string find(const std::string& command) {
  
  std::string path_env{};
  if(const auto p = std::getenv("PATH"); p) path_env = p;
  else return "";

  size_t start = 0;
  for(size_t start = 0; start < path_env.size();) {
    size_t end = path_env.find_first_of(":", start);
    if (end == std::string::npos) end = path_env.size();

    std::string dir = path_env.substr(start, end - start);

    std::filesystem::path p = dir;
    p /= command;

    if (std::filesystem::exists(p) && is_exec(p.string())) {
      return p.string();
    }
    start = end + 1;
  }

  return "";
}

void exec_type(const std::string &command) {
  if (is_builtin(command))
    std::cout << command << " is a shell builtin" << std::endl;
  else if (auto path = find(command); path != "")
    std::cout << command << " is " << path << std::endl;
  else
    std::cout << command << ": not found" << std::endl;
}

void exec_builtin(const std::string &command) {
  if (command.starts_with("exit ")) exit(0);
  else if (command.starts_with("echo ")) std::cout << command.substr(5) << std::endl;
  else if (command.starts_with("type ")) exec_type(command.substr(5));
}

void exec(const std::string &command, const std::string &args = "") {
  if (is_builtin(command)) return exec_builtin(command);

  std::cout << command << ": command not found" << std::endl;
}
} // namespace shell
