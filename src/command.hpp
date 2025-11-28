#pragma once

#include "logger.hpp"
#include "utils.hpp"

#include <string>
#include <unistd.h>
#include <vector>

namespace shell {
    class Command {
      public:
        Command(const std::string& name, const std::string& type, const std::vector<std::string>& args = {},
                int stdin = STDIN_FILENO, int stdout = STDOUT_FILENO, int stderr = STDERR_FILENO)
            : name(name),
              type(type),
              args(args),
              stdin(stdin),
              stdout(stdout),
              stderr(stderr),
              saved_stdin(stdin),
              saved_stdout(stdout),
              saved_stderr(stderr) {};
        static std::shared_ptr<Command> get_command(const std::vector<std::string>& args = {});

        virtual void execute() = 0;
        virtual std::string where_is() = 0;

        const std::string& get_name() {
            return name;
        };
        const std::string& get_type() {
            return type;
        };
        const std::string& get_redirection() {
            return redirection;
        };
        const std::string& get_filename() {
            return filename;
        };

        void set_redirection(const std::string& op) {
            redirection = op;
        };
        void set_filename(const std::string& name) {
            filename = name;
        };

        void close_io() {
            if (stdin != STDIN_FILENO && stdin >= 0) {
                close(stdin);
                stdin = STDIN_FILENO;
            }
            if (stdout != STDOUT_FILENO && stdout >= 0) {
                close(stdout);
                stdout = STDOUT_FILENO;
            }
            if (stderr != STDERR_FILENO && stderr >= 0) {
                close(stderr);
                stderr = STDERR_FILENO;
            }
        }

        void set_stdin(const int fd = STDIN_FILENO) {
            if (stdin != STDIN_FILENO && stdin >= 0) {
                close(stdin);
            }
            stdin = fd;
        };
        void set_stdout(const int fd = STDOUT_FILENO) {
            if (stdout != STDOUT_FILENO && stdout >= 0) {
                close(stdout);
            }
            stdout = fd;
        };
        void set_stderr(const int fd = STDERR_FILENO) {
            if (stderr != STDERR_FILENO && stderr >= 0) {
                close(stderr);
            }
            stderr = fd;
        };

      protected:
        void setup_io() {
            saved_stdin = dup(STDIN_FILENO);
            saved_stdout = dup(STDOUT_FILENO);
            saved_stderr = dup(STDERR_FILENO);
            dup_io();
            close_io();
        }
        void reset_io() {
            reset_stdio(STDIN_FILENO, saved_stdin);
            reset_stdio(STDOUT_FILENO, saved_stdout);
            reset_stdio(STDERR_FILENO, saved_stderr);
        }
        void dup_io() {
            dup2(stdin, STDIN_FILENO);
            dup2(stdout, STDOUT_FILENO);
            dup2(stderr, STDERR_FILENO);
        }
        std::vector<char*> get_argv() {
            std::vector<char*> argv;

            argv.emplace_back(const_cast<char*>(name.c_str()));
            std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const std::string& a) {
                return const_cast<char*>(a.c_str());
            });
            argv.push_back(nullptr);

            return argv;
        }

        const std::string name;
        const std::string type;
        const std::vector<std::string> args;

        int stdin;
        int stdout;
        int stderr;
        int saved_stdin;
        int saved_stdout;
        int saved_stderr;

        std::string redirection{};
        std::string filename{};
    };

} // namespace shell