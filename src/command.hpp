#pragma once

#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

namespace shell {
class Command {
    public:
        Command(const std::string& name, const std::string& type, const std::vector<std::string>&  args = {},
            int stdin = STDIN_FILENO, int stdout = STDOUT_FILENO, int stderr = STDERR_FILENO) :
            name(name), type(type), args(args), stdin(), stdout(), stderr() {};
        static std::shared_ptr<Command> get_command(const std::vector<std::string>&  args = {});

        virtual void execute() = 0;
        virtual std::string where_is() = 0;

        std::string get_name() { return name; };
        std::string get_type() { return type; };

        int get_stdin() { return stdin; };
        int get_stdout() { return stdout; };
        int get_stderr() { return stderr; };
        void set_stdin(const int fd = STDIN_FILENO) { stdin = fd; };
        void set_stdout(const int fd = STDOUT_FILENO) { stdout = fd; };
        void set_stderr(const int fd = STDERR_FILENO) { stderr = fd; };

    protected:
        const std::vector<std::string> args;

        void dup_io() {
            dup2(get_stdin(), STDIN_FILENO);
            dup2(get_stdout(), STDOUT_FILENO);
            dup2(get_stderr(), STDERR_FILENO);
        }
    private:
        const std::string name;
        const std::string type;
        int stdin;
        int stdout;
        int stderr;
};

} // namespace shell