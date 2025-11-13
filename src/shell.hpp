#pragma once

#include "command.hpp"

#include <fcntl.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace shell{

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
    return name == "1>" || name == ">" || name == ">>" || name == "|";
}

void reset_stdout(int saved_stdout) {
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO); // Restore the saved fd to stdout
    close(saved_stdout); // Clean up
}
void set_stdout(const std::string& delimeter, const std::string& filename) {
    int fd = -1;
    if (delimeter == "1>" || delimeter == ">") {
        fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else if (delimeter == ">>") {
        fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    if (fd >= 0) {
        fflush(stdout);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
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
            auto command = Command::get_command(args);
            if (i+1 < tokens.size()) {
                // we look into the next token to find the output file
                // classify delimeter, for now assume its "1>" or ">" or "">>""
                set_stdout(token, tokens.at(++i));
            }

            commands.emplace_back(std::move(command));
            args = {};
        }
    }
    // last element
    if (!args.empty()) commands.emplace_back(std::move(Command::get_command(args)));

    return commands;
}

void run(const std::string &command_line) {

    auto saved_stdout = dup(STDOUT_FILENO);
    for (const auto& c : get_commands(command_line)) {
        c->execute();
    }
    reset_stdout(saved_stdout);

}
} // namespace shell
