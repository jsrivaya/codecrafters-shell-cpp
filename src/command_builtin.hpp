#pragma once

#include "command.hpp"
#include "command_custom.hpp"
#include <cstdlib>
#include <iostream>

namespace shell {
class BuiltinCommand : public Command {
    public:
        BuiltinCommand(const std::string& name) : Command(name, "builtin") { };
        void execute(const std::string& args = "") {
            if (get_name() == "exit") exit(0);
            else if (get_name() == "echo") exec_echo(args); 
            else if (get_name() == "type") exec_type(args);
        }
        std::string where_is() {
            return get_name();
        }
        static bool is_builtin(const std::string& name) {
            return name == "exit" || name == "echo" || name == "type";
        }

    private:
        void exec_echo(const std::string& args) {
            std::cout << args << std::endl;
        }
        void exec_type(const std::string &args) {
            auto c = create(args);

            if (c->get_type() == "builtin") {
                std::cout << c->get_name() << " is a shell builtin" << std::endl;
                return;
            }

            try {
                const auto& p = c->where_is();
                std::cout << c->get_name() << " is " << p << std::endl;
            } catch (const std::runtime_error&) {
                std::cout << c->get_name() << ": not found" << std::endl;
            }

            return;
        }
};

} // namespace shell