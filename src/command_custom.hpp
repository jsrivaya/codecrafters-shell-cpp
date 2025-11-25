#pragma once

#include "command.hpp"
#include "logger.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector> 

namespace shell {
class CustomCommand : public Command {
    public:
        explicit CustomCommand(const std::string& name = "custom", const std::vector<std::string>&  args = {}) : Command(name, "custom", args) { };
        void execute() override {
            shell::Logger::getInstance().debug("CustomCommand::execute", name);
            try {
                if (where_is() != "") {
                    setup_io();

                    execvp(name.c_str(), get_argv().data());

                    reset_io();
                }
            } catch (const std::runtime_error&) {
                std::cerr << name << ": not found" << std::endl;
            }
        }
        std::string where_is() override {
            if(const auto p = std::getenv("PATH"); p) {
                std::string path_env{p};
                for(size_t start = 0; start < path_env.size();) {
                    size_t end = path_env.find_first_of(":", start);
                    if (end == std::string::npos) end = path_env.size();

                    std::filesystem::path path = path_env.substr(start, end - start) + "/" + name;
                    if (std::filesystem::exists(path) && access(path.c_str(), X_OK) == 0) {
                        return path.string();
                    }
                    start = end + 1;
                }
            }
            throw std::runtime_error("not found");
        }
};

} // namespace shell