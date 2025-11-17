#pragma once

#include <memory>
#include <string>
#include <unistd.h>
#include <vector>
#include <fstream>


namespace shell {
class Command {
    public:
        Command(const std::string& name, const std::string& type, const std::vector<std::string>&  args = {},
            int stdin = STDIN_FILENO, int stdout = STDOUT_FILENO, int stderr = STDERR_FILENO) :
            name(name), type(type), args(args), stdin(stdin), stdout(stdout), stderr(stderr) {};
        static std::shared_ptr<Command> get_command(const std::vector<std::string>&  args = {});

        virtual void execute() = 0;
        virtual std::string where_is() = 0;
        virtual bool can_spawn() = 0;

        std::string get_name() { return name; };
        std::string get_type() { return type; };
        std::string get_redirection() { return redirection; };
        std::string get_filename() { return filename; };

        void set_redirection(const std::string& op) { redirection = op; };
        void set_filename(const std::string& name) { filename = name; };
        void set_pipe_read(int fd) {
            pipe_read = fd;
        }
        void set_pipe_write(int fd) {
            pipe_write = fd;
        }
        void close_pipe() {
            if (pipe_read >= 0) { close(pipe_read); }
            if (pipe_write >= 0) { close(pipe_write); }
        }
        void close_io() {
            if (stdin != STDIN_FILENO && stdin >= 0) { close(stdin);}
            if (stdout != STDOUT_FILENO && stdout >= 0) { close(stdout);}
            if (stderr != STDERR_FILENO && stderr >= 0) { close(stderr);}
        }

        int get_stdin() { return stdin; };
        int get_stdout() { return stdout; };
        int get_stderr() { return stderr; };
        void set_stdin(const int fd = STDIN_FILENO) { stdin = fd; };
        void set_stdout(const int fd = STDOUT_FILENO) { stdout = fd; };
        void set_stderr(const int fd = STDERR_FILENO) { stderr = fd; };

        std::string get_commandline() {
            auto line = name;
            for (const auto& a : args) {
                line += " " + a;
            }
            return line;
        }

    protected:
        void dup_io() {
            dup2(stdin, STDIN_FILENO);
            dup2(stdout, STDOUT_FILENO);
            dup2(stderr, STDERR_FILENO);
        }
        std::vector<char*> get_argv() {
            std::vector<char*> argv;
            argv.emplace_back(const_cast<char*>(name.c_str()));
            for (const auto& a : args)
                argv.emplace_back(const_cast<char*>(a.c_str()));

            argv.push_back(nullptr);
            return argv;
        }

        const std::string name;
        const std::string type;
        const std::vector<std::string> args;
        int stdin;
        int stdout;
        int stderr;

        std::string redirection{};
        std::string filename{};
        int pipe_read = -1;
        int pipe_write = -1;
};

} // namespace shell