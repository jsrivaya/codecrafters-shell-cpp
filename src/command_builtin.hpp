#pragma once

#include "command.hpp"
#include "command_custom.hpp"

#include <cstdlib>
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
                    if (get_name() == "cd") cd(args);
                    else if (get_name() == "echo") spawn(echo, args);
                    else if (get_name() == "exit") exit(0);
                    else if (get_name() == "pwd") pwd();
                    else if (get_name() == "type") type(args);
                }
            } catch (const std::runtime_error&) {
                std::cerr << get_name() << ": not found" << std::endl;
            }
        }
        std::string where_is() {
            return get_name();
        }
        static bool is_builtin(const std::string& name) {
            return name == "cd" || name == "echo" || name == "exit" || name == "pwd" || name == "type";
        }
        static std::vector<std::string> get_all_commands() {
            return {"cd", "echo", "exit", "pwd", "type"};
        }
    private:
        static void cd(const  std::vector<std::string>& path) {
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
        static void echo(const std::vector<std::string>& args) {
            for (size_t i = 0; i < args.size(); ++i) {
                std::cout << args[i];
                if (i < args.size() - 1) std::cout << " ";
            }
            std::cout << std::endl;
            std::cout.flush();
        }
        static void pwd() {
            std::cout << std::filesystem::current_path().string() << std::endl;
        }
        static void type(const  std::vector<std::string>& args) {
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
        using FuncPtr = void(*)(const std::vector<std::string>&  args);
        void spawn(FuncPtr func_ptr, const std::vector<std::string>&  args) {
            pid_t pid = fork();
            if (pid == 0) {

                func_ptr(args);
                std::exit(1);
            } else if (pid > 0) {
                // Parent waits
                wait(nullptr);
            } else {
                std::cerr << get_name() << ": fork failed" << std::endl;
            }
        }
        std::vector<char*> get_argv(const std::vector<std::string>&  args = {}) {
            std::vector<char*> argv;
            argv.emplace_back(const_cast<char*>(get_name().c_str()));
            for (const auto& a : args)
                argv.emplace_back(const_cast<char*>(a.c_str()));

            argv.push_back(nullptr);
            return argv;
        }
};

} // namespace shell