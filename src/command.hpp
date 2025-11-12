#pragma once

#include <memory>
#include <string>
#include <vector>

namespace shell {
class Command {
    public:
        Command(const std::string& name, const std::string& type, const std::vector<std::string>&  args = {},
            int stdin = 0, int stdout = 1, int stderr = 2) :
            name_(name), type_(type), args(args), stdin_(), stdout_(), stderr_() {};
        static std::shared_ptr<Command> get_command(const std::vector<std::string>&  args = {});

        virtual void execute() = 0;
        virtual std::string where_is() = 0;

        std::string get_name() { return name_; };
        std::string get_type() { return type_; };

    protected:
        const std::vector<std::string> args;

    private:
        const std::string name_;
        const std::string type_;
        const int stdin_;
        const int stdout_;
        const int stderr_;
};

} // namespace shell