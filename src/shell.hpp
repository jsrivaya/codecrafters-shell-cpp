#pragma once

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace shell{

bool is_builtin(const std::string& command) {
    return command.starts_with("exit") ||
           command.starts_with("echo") || 
           command.starts_with("type");
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

    if (std::filesystem::exists(p) && access(p.c_str(), X_OK) == 0) {
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

void exec_echo(const std::string& args){
    std::cout << args << std::endl;
}

void exec_builtin(const std::vector<std::string>& command) {
  if (command.at(0) == "exit") exit(0);
  else if (command.at(0) == "echo") exec_echo(command.at(1)); 
  else if (command.at(0) == "type") exec_type(command.at(1));
}

void exec_custom(const std::vector<std::string>& command) {
  if (auto path = find(command.at(0)); path != "")
    std::system((command.at(0) + " " + command.at(1)).c_str());
  else
    std::cout << command.at(0) << ": not found" << std::endl;
}

std::vector<std::string> get_args (const std::string& request) {
  std::istringstream iss(request);
  std::string args;
  std::string command;

  iss >> command;
  std::getline(iss, args);
  args = args == "" ? "" : args.substr(1) ;
  
  return std::vector<std::string>{command, args};
}

void exec(const std::string &command) {
  std::vector<std::string> args = get_args(command);

  if (is_builtin(args.at(0))) return exec_builtin(args);

  return exec_custom(args);
}
} // namespace shell
