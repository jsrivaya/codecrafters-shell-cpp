#pragma once

#include "command.hpp"

#include <sstream>
#include <utility>

namespace shell{

std::pair<std::string, std::string> get_args (const std::string& request) {
  std::istringstream iss(request);
  std::string args;
  std::string command;

  iss >> command;
  std::getline(iss, args);
  args = args == "" ? "" : args.substr(1) ;
  
  return {command, args};
}

void run(const std::string &command_line) {
  auto [name, args] = get_args(command_line);

  auto c = Command::create(name);
  c->execute(args);
}
} // namespace shell
