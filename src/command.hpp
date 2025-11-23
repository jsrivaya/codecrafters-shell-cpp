#pragma once

#include "logger.hpp"

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

namespace shell {
class Command {
    public:
        Command(const std::string& name, const std::string& type, const std::vector<std::string>&  args = {},
            int stdin = STDIN_FILENO, int stdout = STDOUT_FILENO, int stderr = STDERR_FILENO) :
            name(name), type(type), args(args), stdin(stdin), stdout(stdout), stderr(stderr), pipe_read(-1), pipe_write(-1) {};
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

        void close_io() {
            if (stdin != STDIN_FILENO && stdin >= 0) { close(stdin); stdin = STDIN_FILENO;}
            if (stdout != STDOUT_FILENO && stdout >= 0) { close(stdout);  stdout = STDOUT_FILENO; }
            if (stderr != STDERR_FILENO && stderr >= 0) { close(stderr);  stderr = STDERR_FILENO; }
        }

        int get_stdin() { return stdin; };
        int get_stdout() { return stdout; };
        int get_stderr() { return stderr; };

        void set_stdin(const int fd = STDIN_FILENO) { 
            if (stdin != STDIN_FILENO && stdin >= 0) { close(stdin);}
            stdin = fd;
        };
        void set_stdout(const int fd = STDOUT_FILENO) {
            if (stdout != STDOUT_FILENO && stdout >= 0) { close(stdout);}
            stdout = fd;
        };
        void set_stderr(const int fd = STDERR_FILENO) {
            if (stderr != STDERR_FILENO && stderr >= 0) { close(stderr); }
            stderr = fd;
        };

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
        void reset_stdio(int io_fileno, int saved_stdio) {
            dup2(saved_stdio, io_fileno); // Restore the saved fd to stdout
            close(saved_stdio); // Clean up
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
        int pipe_read;
        int pipe_write;

        std::string redirection{};
        std::string filename{};
};

} // namespace shell