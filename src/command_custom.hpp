#pragma once

#include "command.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector> 

namespace shell {
class CustomCommand : public Command {
    public:
        CustomCommand(const std::string& name, const std::vector<std::string>&  args = {}) : Command(name, "custom", args) { };
        void execute() {
            shell::Logger::getInstance().debug("CustomCommand::execute", name);
            try {
                if (where_is() != "") {
                    auto saved_stdin = dup(STDIN_FILENO);
                    auto saved_stdout = dup(STDOUT_FILENO);
                    auto saved_stderr = dup(STDERR_FILENO);
                    auto pid = getpid();
                    dup_io();
                    close_io();

                    execvp(name.c_str(), get_argv().data());

                    reset_stdio(STDIN_FILENO, saved_stdin);
                    reset_stdio(STDOUT_FILENO, saved_stdout);
                    reset_stdio(STDERR_FILENO, saved_stderr);
                }
            } catch (const std::runtime_error&) {
                std::cerr << name << ": not found" << std::endl;
            }
        }
        std::string where_is() {
            if(const auto p = std::getenv("PATH"); p) {
                std::string path_env{p};
                for(size_t start = 0; start < path_env.size();) {
                    size_t end = path_env.find_first_of(":", start);
                    if (end == std::string::npos) end = path_env.size();

                    std::filesystem::path p = path_env.substr(start, end - start) + "/" + name;
                    if (std::filesystem::exists(p) && access(p.c_str(), X_OK) == 0) {
                        return p.string();
                    }
                    start = end + 1;
                }
            }
            throw std::runtime_error("Command not found");   
        }
        bool can_spawn() {
            return true;
        }
};

} // namespace shell