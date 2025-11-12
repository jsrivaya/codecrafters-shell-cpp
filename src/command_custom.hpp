#pragma once

#include "command.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector> 

namespace shell {
class CustomCommand : public Command {
    public:
        CustomCommand(const std::string& name) : Command(name, "custom") { };
        void execute(const std::vector<std::string>&  args = {}) {
            try {
                if (where_is() != "") {
                    auto argv = get_argv(args);
                    spawn(argv);
                }
            } catch (const std::runtime_error&) {
                std::cerr << get_name() << ": not found" << std::endl;
            }
        }
        std::string where_is() {
            if(const auto p = std::getenv("PATH"); p) {
                std::string path_env{p};
                for(size_t start = 0; start < path_env.size();) {
                    size_t end = path_env.find_first_of(":", start);
                    if (end == std::string::npos) end = path_env.size();

                    std::filesystem::path p = path_env.substr(start, end - start) + "/" + get_name();
                    if (std::filesystem::exists(p) && access(p.c_str(), X_OK) == 0) {
                        return p.string();
                    }
                    start = end + 1;
                }
            }
            throw std::runtime_error("Command not found");   
        }
    private:
        void spawn(const std::vector<char*>&  argv = {}) {
            pid_t pid = fork();
            if (pid == 0) {
                execvp(get_name().c_str(), argv.data());
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