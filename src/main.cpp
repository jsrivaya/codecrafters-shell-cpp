#include <iostream>
#include <string>

#include "shell.hpp"

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  do {
    std::cout << "$ ";
    std::string command{};
    std::getline(std::cin, command);

    shell::exec(command);
  } while(true);

  exit(0);
}
