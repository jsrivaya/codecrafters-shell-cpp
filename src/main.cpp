#include <iostream>
#include <string>

#include "shell.hpp"

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
