#pragma once

#include <memory>
#include <string>
#include <vector>

namespace shell {
class Command {
    public:
        Command(const std::string& name, const std::string& type, const std::vector<std::string>&  args = {}) :
            name(name), type(type), args(args) {};
        static std::shared_ptr<Command> get_command(const std::string& name, const std::vector<std::string>&  args = {});

        virtual void execute() = 0;
        virtual std::string where_is() = 0;

        std::string get_name() { return name; };
        std::string get_type() { return type; };

    protected:
        const std::vector<std::string> args;

    private:
        const std::string name;
        const std::string type;
};

} // namespace shell