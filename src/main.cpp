#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::unordered_set<std::string> commands{"exit", "echo", "type"};

  while(true) {
    std::cout << "$ ";
    std::string command{};
    std::getline(std::cin, command);

    if(command.starts_with("exit")) {
      exit(0);
    } else if (command.starts_with("echo")) {
      std::cout << command.substr(5) << std::endl;
    } else if (command.starts_with("type")) {
      if(commands.find(command.substr(4)) != commands.end()) std::cout << command.substr(5) << ": not found" << std::endl;
      else std::cout << command.substr(4) << " is a shell builtin" << std::endl;
    } else {
      std::cout << command << ": command not found" << std::endl;
    }
  }

}
