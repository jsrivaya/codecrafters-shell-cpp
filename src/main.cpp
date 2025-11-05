#include <iostream>
#include <string>

#include "shell.hpp"

std::vector<std::string> get_args (const std::string& command) {
  std::istringstream iss(command);
  std::vector<std::string> tokens;
  std::string token;
  while (iss >> token) {
      tokens.emplace_back(token);
  }
  return tokens;
}
  

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while(true) {
    std::cout << "$ ";
    std::string command{};
    std::getline(std::cin, command);

    shell::exec(command);
  }

  exit(0);
}
