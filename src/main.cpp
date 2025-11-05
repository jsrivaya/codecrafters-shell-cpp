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
      std::vector<std::string> tokens{};
      std::istringstream iss(command);
      std::string token;
      while (iss >> token) {
          tokens.push_back(token);
          if(token != "echo") std::cout << token << std::endl;
      }
    }

    std::cout << command << ": command not found" << std::endl;
  }

}
