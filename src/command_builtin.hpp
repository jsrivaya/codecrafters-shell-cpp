#pragma once

#include "command.hpp"
#include "command_custom.hpp"
#include "history.hpp"

#include <charconv>
#include <filesystem>
#include <iostream>
#include <fstream>

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
                    if (name == "cd") cd(args);
                    else if (name == "echo") echo(args);
                    else if (name == "exit") exit_shell(args);
                    else if (name == "history") history(args);
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
            return name == "cd" || name == "echo" || name == "exit" || name == "history" || name == "pwd" || name == "type";
        }
        static std::vector<std::string> get_all_commands() {
            return {"cd", "echo", "exit", "pwd", "type"};
        }
        bool can_spawn() {
            return !(name == "cd" || name == "exit");
        }
    private:
        void cd(const  std::vector<std::string>& path = {}) {
            if (auto p = std::getenv("HOME"); path.empty() || (path.at(0) == "~" && p))
                std::filesystem::current_path(std::string{p});
            else if (path.size() > 1)
                std::cerr << "cd: : too many arguments" << std::endl;
            else if (!path.empty() && std::filesystem::exists(std::filesystem::path(path.at(0))))
                std::filesystem::current_path(path.at(0));
            else
                std::cerr << "cd: " + path.at(0) + ": No such file or directory" << std::endl;
        }
        void echo(const std::vector<std::string>& args = {}) {
            for (size_t i = 0; i < args.size(); ++i) {
                std::cout << args[i];
                if (i < args.size() - 1) std::cout << " ";
            }
            std::cout << std::endl;
        }
        void exit_shell(const std::vector<std::string>& args = {}) {
            exit(0);
        }
        void history(const std::vector<std::string>& args = {}) {
            if(args.empty())
                return History::getInstance().print_last();

            int number;
            auto result = std::from_chars(args.at(0).data(), args.at(0).data() + args.at(0).size(), number);
            if (result.ec == std::errc()) {
                History::getInstance().print_last(number);
            } else {
                std::cerr << "Invalid number" << std::endl;
            }
        }
        void pwd(const std::vector<std::string>& args = {}) {
            if (!args.empty()) std::cerr << "pwd: : too many arguments" << std::endl;
            std::cout << std::filesystem::current_path().string() << std::endl;
        }
        void type(const  std::vector<std::string>& args = {}) {
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