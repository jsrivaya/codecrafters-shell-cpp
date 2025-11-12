#pragma once

#include "command.hpp"
#include "command_custom.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace shell {
class BuiltinCommand : public Command {
    public:
        BuiltinCommand(const std::string& name) : Command(name, "builtin") { };
        void execute(const  std::vector<std::string>&  args = {}) {
            // Builtin commands like 'cd' need to modify the shell's own state
            // others like: echo, exit, pwd, type are simple enough that forking would cause too much overhead
            if (get_name() == "cd") cd(args); 
            else if (get_name() == "echo") echo(args); 
            else if (get_name() == "exit") exit(0);
            else if (get_name() == "pwd") pwd();
            else if (get_name() == "type") type(args);
        }
        std::string where_is() {
            return get_name();
        }
        static bool is_builtin(const std::string& name) {
            return name == "cd" || name == "echo" || name == "exit" || name == "pwd" || name == "type";
        }

    private:
        void cd(const  std::vector<std::string>& path) {
            if (auto p = std::getenv("HOME"); path.empty() ||
                (path.size() == 1 && path.at(0) == "~" && p))
                std::filesystem::current_path(std::string{p});
            else if (path.size() > 1)
                std::cerr << "cd: : too many arguments" << std::endl;
            else if (std::filesystem::exists(std::filesystem::path(path.at(0))))
                std::filesystem::current_path(path.at(0));
            else
                std::cerr << "cd: " + path.at(0) + ": No such file or directory" << std::endl;
        }
        void echo(const std::vector<std::string>& args) {
            for (size_t i = 0; i < args.size(); ++i) {
                std::cout << args[i];
                if (i < args.size() - 1) std::cout << " ";
            }
            std::cout << std::endl;
        }
        void pwd() {
            std::cout << std::filesystem::current_path().string() << std::endl;
        }
        void type(const  std::vector<std::string>& args) {
            for (const auto& a : args) {
                auto command = get_command(a);
                if (command->get_type() == "builtin") {
                    std::cout << command->get_name() << " is a shell builtin" << std::endl;
                    return;
                }
                try {
                    const auto& path = command->where_is();
                    std::cout << command->get_name() << " is " << path << std::endl;
                } catch (const std::runtime_error&) {
                    std::cerr << command->get_name() << ": not found" << std::endl;
                }
            }
        }
};

} // namespace shell