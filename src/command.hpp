#pragma once

#include <filesystem>
#include <string>
#include <unistd.h>

namespace shell {
class Command {
    public:
        static std::unique_ptr<Command>  create(const std::string &name);
        Command(const std::string& name, const std::string& type) : name(name), type(type) {};
        virtual void execute(const std::string& args) = 0;
        virtual std::string where_is() = 0;

        std::string get_name() { return name; };
        std::string get_type() { return type; };

    private:
        const std::string name;
        const std::string type;
};

} // namespace shell