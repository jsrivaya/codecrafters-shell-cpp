#pragma once

#include "command.hpp"

#include <sstream>
#include <vector>

namespace shell{

std::vector<std::string> get_args (const std::string& request) {
  std::istringstream iss(request);
  std::string args;
  std::string command;

  iss >> command;
  std::getline(iss, args);
  args = args == "" ? "" : args.substr(1) ;
  
  return std::vector<std::string>{command, args};
}

void run(const std::string &command_line) {
  std::vector<std::string> args = get_args(command_line);

  auto c = Command::create(args.at(0));
  c->execute(args.size() > 1 ? args[1] : "");
}
} // namespace shell
