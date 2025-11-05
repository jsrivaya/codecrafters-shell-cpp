#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <filesystem>

#include <cstdlib>
#include <unistd.h>

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

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::unordered_set<std::string> commands{"exit", "echo", "type"};

  auto is_builtin = [commands](const std::string& command) {
    return commands.find(command.substr(5)) != commands.end();
  };

  while(true) {
    std::cout << "$ ";
    std::string command{};
    std::getline(std::cin, command);

    if(command.starts_with("exit")) {
      exit(0);
    } else if (command.starts_with("echo")) {
      std::cout << command.substr(5) << std::endl;
    } else if (command.starts_with("type")) {
      if(is_builtin(command)) std::cout << command.substr(5) << " is a shell builtin" << std::endl;
      else {
        if (auto path = find(command.substr(5)); path != "") std::cout << command.substr(5) << " is " << path << std::endl;
        else std::cout << command.substr(5) << ": not found" << std::endl;
      }
    } else {
      std::cout << command << ": command not found" << std::endl;
    }
  }

}
