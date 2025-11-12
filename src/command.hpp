#pragma once

#include <memory>
#include <string>
#include <vector>

namespace shell {
class Command {
    public:
        Command(const std::string& name, const std::string& type) : name(name), type(type) {};
        static std::shared_ptr<Command> get_command(const std::string& name);

        virtual void execute(const std::vector<std::string>&  args = {}) = 0;
        virtual std::string where_is() = 0;

        std::string get_name() { return name; };
        std::string get_type() { return type; };

    private:
        const std::string name;
        const std::string type;
};

} // namespace shell