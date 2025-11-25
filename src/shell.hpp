#pragma once

#include "command.hpp"
#include "command_builtin.hpp"
#include "command_line.hpp"
#include "history.hpp"
#include "logger.hpp"
#include "shell_parser.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <vector>

namespace shell{

/**
 * @brief Initializes shell prerequisites
 * 
 */
void init () {
    // initialize readline library
    rl_init();
}

/**
 * @brief Get the pipeline object
 * 
 * @param command_line string received from starndard input
 * @return std::vector<std::shared_ptr<Command>> 
 */
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

pid_t spawn(std::shared_ptr<Command> command) {
    pid_t pid = fork();
    if (pid == 0) {
        command->execute();
        std::exit(1);
    }
    return pid;
}

/**
 * @brief Executes one single command
 * 
 * @param command the command
 * @return pid_t either shell pid if builtin command or custom command process pid
 */
pid_t execute(std::shared_ptr<Command> command) {
    shell::Logger::getInstance().debug("execute()", command->get_name());
    pid_t pid = -1;
    if(BuiltinCommand::is_builtin(command->get_name())) {
        command->execute(); // execute in the shell process
        pid = getpid();
    } else{
        pid = spawn(command); // execute in child process
        command->close_io();
    }
    return pid;
}

/**
 * @brief Set the up and prepare pipeline object for execution
 * 
 * @param pipeline the list of Commands to prepare for execution
 */
void setup_pipeline(std::vector<std::shared_ptr<Command>> pipeline) {
    for (size_t i = 0; i<pipeline.size(); ++i) {
        auto command = pipeline.at(i);
        if (command->get_redirection() == "1>" || command->get_redirection() == ">") {
            command->set_stdout(
                open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
            );
        } else if (command->get_redirection() == "1>>" || command->get_redirection() == ">>") {
            command->set_stdout(
                open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
            );
        } else if (command->get_redirection() == "2>") {
            command->set_stderr(
                open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
            );
        } else if (command->get_redirection() == "2>>") {
            command->set_stderr(
                open(command->get_filename().c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
            );
        } else if(command->get_redirection() == "|") {
            auto next_command = pipeline.at(i+1);
            int pipe_fd[2];
            pipe(pipe_fd);

            command->set_stdout(pipe_fd[1]); // writer side
            next_command->set_stdin(pipe_fd[0]); // reader side
        }
    }
}

/**
 * @brief Executes a list of Commands
 * 
 * @param pipeline the list of Commands
 */
void run_pipeline(std::vector<std::shared_ptr<Command>> pipeline) {
    std::vector<pid_t> pids;
    for (auto& cmd : pipeline) {
        pid_t pid = execute(cmd);
        pids.push_back(pid);
    }

    // Now wait for all children
    for (pid_t pid : pids) {
        int status;
        if (pid > 0) {  // Only parent waits
            waitpid(pid, &status, 0);
        }
    }

    // After launching all children and calling waitpid() for each:
    for (auto& cmd : pipeline) {
        cmd->close_io();  // Now safe to close pipe FDs
    }
}

/**
 * @brief Process a command line and executes the different commands and operators
 * 
 * @param command_line the standard input command line string
 */
void run(const std::string &command_line) {

    auto saved_stdin = dup(STDIN_FILENO);
    auto saved_stdout = dup(STDOUT_FILENO);
    auto saved_stderr = dup(STDERR_FILENO);

    try {
        History::getInstance().log_command_line(command_line);
        const auto pipeline = get_commands(command_line);
        setup_pipeline(pipeline);
        run_pipeline(pipeline);
    } catch (const std::exception& e) {
        Logger::getInstance().error(e.what());
        History::getInstance().persist_history();
    }

    reset_stdio(STDERR_FILENO, saved_stderr);
    reset_stdio(STDOUT_FILENO, saved_stdout);
    reset_stdio(STDIN_FILENO, saved_stdin);
}
} // namespace shell
