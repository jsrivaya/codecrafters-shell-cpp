#pragma once

#include "command.hpp"
#include "command_custom.hpp"

#include <filesystem>
#include <iostream>

namespace shell {
class BuiltinCommand : public Command {
    public:
        BuiltinCommand(const std::string& name, const std::vector<std::string>&  args = {}) : Command(name, "builtin", args) { };
        void execute() {
            try {
                if (where_is() != "") {
                    // Builtin commands like 'cd' need to modify the shell's own state
                    // others like: echo, exit, pwd, type are simple enough that forking would cause too much overhead
                    dup_io();
                    close_io();
                    close_pipe();
                    if (name == "cd") cd(args);
                    else if (name == "echo") echo(args);
                    else if (name == "exit") exit(0);
                    else if (name == "pwd") pwd(args);
                    else if (name == "type") type(args);
                }
            } catch (const std::runtime_error&) {
                std::cerr << name << ": not found" << std::endl;
            }
        }
        std::string where_is() {
            return name;
        }
        static bool is_builtin(const std::string& name) {
            return name == "cd" || name == "echo" || name == "exit" || name == "pwd" || name == "type";
        }
        static std::vector<std::string> get_all_commands() {
            return {"cd", "echo", "exit", "pwd", "type"};
        }
    private:
        static void cd(const  std::vector<std::string>& path = {}) {
            if (auto p = std::getenv("HOME"); path.empty() || (path.at(0) == "~" && p))
                std::filesystem::current_path(std::string{p});
            else if (path.size() > 1)
                std::cerr << "cd: : too many arguments" << std::endl;
            else if (!path.empty() && std::filesystem::exists(std::filesystem::path(path.at(0))))
                std::filesystem::current_path(path.at(0));
            else
                std::cerr << "cd: " + path.at(0) + ": No such file or directory" << std::endl;
        }
        static void echo(const std::vector<std::string>& args = {}) {
            for (size_t i = 0; i < args.size(); ++i) {
                std::cout << args[i];
                if (i < args.size() - 1) std::cout << " ";
            }
            std::cout << std::endl;
        }
        static void pwd(const std::vector<std::string>& args = {}) {
            if (!args.empty()) std::cerr << "pwd: : too many arguments" << std::endl;
            std::cout << std::filesystem::current_path().string() << std::endl;
        }
        static void type(const  std::vector<std::string>& args = {}) {
            for (const auto& a : args) {
                if (BuiltinCommand::is_builtin(a)) {
                    std::cout << a << " is a shell builtin" << std::endl;
                    return;
                }
                try {
                    auto command = get_command({a});
                    const auto& path = command->where_is();
                    std::cout << a << " is " << path << std::endl;
                } catch (const std::runtime_error&) {
                    std::cerr << a << ": not found" << std::endl;
                }
            }
        }
};

} // namespace shell