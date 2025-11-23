#include "shell.hpp"
#include "logger.hpp"

int main() {

  // 1. Initialize logger
  shell::Logger::getInstance().enable(shell::Logger::Level::INFO);

  // 2. Initialize shell
  shell::init();

  do {
    // 3. Read command line and Run shell
    shell::run(shell::read_line());

  } while(true);

  exit(0);
}
