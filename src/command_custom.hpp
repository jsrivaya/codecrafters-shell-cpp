#pragma once

#include "command.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unistd.h>

namespace shell {
class CustomCommand : public Command {
    public:
        CustomCommand(const std::string& name) : Command(name, "custom") { };
        void execute(const std::string& args = "") {
            try {
                if (where_is() != "")
                    std::system((get_name() + " " + args).c_str());
            } catch (const std::runtime_error&) {
                std::cout << get_name() << ": not found" << std::endl;
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
};

} // namespace shell