#include <iostream>
#include <string>
#include <sstream>
#include <vector>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while(true) {
    std::cout << "$ ";
    std::string command{};
    std::getline(std::cin, command);

    if(command.starts_with("exit")) {
      exit(0);
    } else if (command.starts_with("echo")) {
      std::cout << command.substr(5);
    }

    std::cout << command << ": command not found" << std::endl;
  }

}
