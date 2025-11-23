#include "shell.hpp"
#include "logger.hpp"

#include <iostream>


int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  shell::Logger::getInstance().enable(shell::Logger::Level::INFO);

  shell::init();
  
  do {
    shell::run(shell::read_line());

  } while(true);

  exit(0);
}
