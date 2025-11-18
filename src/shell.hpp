#pragma once

#include "command.hpp"
#include "command_builtin.hpp"
#include "history.hpp"
#include "logger.hpp"

#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <utility>
#include <vector>
#include <deque>

#include <unistd.h>

namespace shell{

    static int n_proc = 0;
// std::pair<std::string, std::vector<std::string>> get_args(const std::string& s) {
// return: { arg0, arg1, arg2, arg3 }
// (e.g {cat, /tmp/file.txt, >, myfile.txt})
std::vector<std::string> get_tokens(const std::string& s) {
    std::string current_token{};
    std::vector<std::string> tokens{};
    bool in_quotes = false;
    bool in_double_quotes = false;
    bool prev_space = false;
    bool in_escaped = false;

    for(const auto& c : s) {
        if(in_escaped) {
            // Inside double quotes: \ only escapes $, ', ", \, or newline — otherwise it's literal
            if(in_double_quotes && (c !='$' && c != '`' && c != '"' && c != '\\')) {
                current_token += "\\"; // we preseve backslash
            }
            current_token += c;
            in_escaped = false;
            continue;
        }
        if (in_quotes && c != '\'') {
            current_token += c;
            continue;
        }
        if (!in_quotes && c == '\\') {
            // Outside quotes: \ preserves the literal value of the next character
            in_escaped = !in_escaped;
            continue;
        }
        if (!in_double_quotes && c == '\'') {
            in_quotes = !in_quotes;
            prev_space = false;
            continue;
        }
        if (!in_quotes && c == '"') {
            in_double_quotes = !in_double_quotes;
            in_quotes = false;
            prev_space = false;
            continue;
        }
        if (in_double_quotes && c != '"') {
            current_token += c;
            continue;
        }
        if ((c == ' ' || c == '\t') && !in_quotes && !in_double_quotes) {
            if(prev_space) continue;

            prev_space = true;
            tokens.emplace_back(current_token);
            current_token = {};
            continue;
        }
        current_token += c;
        prev_space = false;
    }
    if(!current_token.empty()) tokens.emplace_back(current_token);

    return tokens;
}

bool is_delimeter(const std::string& name) {
    return name == "1>" || name == ">" || name == ">>" || name == "1>>" ||
            name == "2>" || name == "2>>" || name == "|" || name == "&|";
}

void reset_stdio(int io_fileno, int saved_stdio) {
    dup2(saved_stdio, io_fileno); // Restore the saved fd to stdout
    close(saved_stdio); // Clean up
}

std::vector<std::shared_ptr<Command>> get_commands(const std::string& command_line) {
    std::vector<std::shared_ptr<Command>> commands{};
    std::vector<std::string> args{};

    const auto& tokens = get_tokens(command_line);
    for (int i = 0; i < tokens.size(); ++i) {
        auto& token = tokens.at(i);
        if (!is_delimeter(token)) {
            args.emplace_back(std::move(token));
        } else {
            std::shared_ptr<Command> command = Command::get_command(args);
            if (i+1 < tokens.size()) {
                // we look into the next token to find the output file
                // it's the user reponsability to pass a file or command after delimeter
                command->set_redirection(token);
                command->set_filename(tokens.at(i+1));
                if(token != "|") ++i;
            }

            commands.emplace_back(std::move(command));
            args = {};
        }
    }
    // last element
    if (!args.empty()) commands.emplace_back(std::move(Command::get_command(args)));

    return commands;
}

void spawn(std::shared_ptr<Command> command) {
    pid_t pid = fork();
    if (pid == 0) {
        command->execute();
        std::exit(1);
    }
    ++n_proc;
}

void execute(std::shared_ptr<Command> command) {
    shell::Logger::getInstance().debug("execute", command->get_name());
    if(BuiltinCommand::is_builtin(command->get_name())) {
        command->execute(); // execute in the shell process
    } else{
        spawn(command); // execute in child process
    }

    // parent: close up pipe descriptors if open
    command->close_io();
}

void setup_pipeline(std::vector<std::shared_ptr<Command>> pipeline) {
    for (size_t i = 0; i<pipeline.size(); ++i) {
        auto command = pipeline.at(i);
        if (command->get_redirection() == "1>" || command->get_redirection() == ">") {
            command->set_stdout(open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 644));
        } else if (command->get_redirection() == "1>>" || command->get_redirection() == ">>") {
            command->set_stdout(open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_APPEND, 644));
        } else if (command->get_redirection() == "2>") {
            command->set_stderr( open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 644));
        } else if (command->get_redirection() == "2>>") {
            command->set_stderr(open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_APPEND, 644));
        } else if(command->get_redirection() == "|") {
            auto next_command = pipeline.at(i+1);
            int pipe_fd[2];
            pipe(pipe_fd);

            command->set_stdout(pipe_fd[1]); // writer side
            next_command->set_stdin(pipe_fd[0]); // reader side
        }
    }
}

void wait_for_commands(/*size_t n_proc*/) {
    shell::Logger::getInstance().debug("wait_for_commands");
    for (size_t i = 0; i<n_proc; ++i) {
        wait(nullptr);
    }
    n_proc = 0;
}

void close_pipes(std::vector<std::shared_ptr<Command>> pipeline) {
    shell::Logger::getInstance().debug("close_pipes", std::to_string(pipeline.size()));
    for(const auto& command : pipeline) {
        command->close_io();
    }
}

void close_pipeline(std::vector<std::shared_ptr<Command>> pipeline) {
    shell::Logger::getInstance().debug("close_pipeline", std::to_string(pipeline.size()));
    close_pipes(pipeline);

    wait_for_commands(/*pipeline.size()*/);
}

void wait_and_exit(/*size_t n_proc*/) {
    wait_for_commands(/*n_proc*/);
    exit(0);
}

void run_pipeline(std::vector<std::shared_ptr<Command>> pipeline) {
    std::vector<std::shared_ptr<Command>> builtin_commands{};
    for (const auto& command : pipeline) {
        if(BuiltinCommand::is_builtin(command->get_name())) {
            builtin_commands.emplace_back(command);
            continue;
        }
        // if(command->get_name() == "exit") {
        //     History::getInstance().persist_history();
        //     wait_and_exit(/*pipeline.size()*/);
        // }
        execute(command);
    }

    for (const auto& command : builtin_commands)
        execute(command);

}

void run(const std::string &command_line) {

    auto saved_stdout = dup(STDOUT_FILENO);
    auto saved_stderr = dup(STDERR_FILENO);

    try {
        History::getInstance().log_command_line(command_line);
        const auto pipeline = get_commands(command_line);
        setup_pipeline(pipeline);
        run_pipeline(pipeline);
        close_pipeline(pipeline);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        History::getInstance().persist_history();
    }

    reset_stdio(STDERR_FILENO, saved_stderr);
    reset_stdio(STDOUT_FILENO, saved_stdout);
}
} // namespace shell
