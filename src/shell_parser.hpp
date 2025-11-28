#pragma once

#include <string>
#include <vector>

namespace shell {

    /**
     * @brief Get command line tokens
     *
     * @param s
     * @return std::vector<std::string>  (e.g {cat, /tmp/file.txt, >, myfile.txt})
     */
    std::vector<std::string> get_tokens(const std::string& s) {
        std::string current_token{};
        std::vector<std::string> tokens{};
        bool in_quotes = false;
        bool in_double_quotes = false;
        bool prev_space = false;
        bool in_escaped = false;

        for (const auto& c : s) {
            if (in_escaped) {
                // Inside double quotes: \ only escapes $, ', ", \, or newline — otherwise it's literal
                if (in_double_quotes && (c != '$' && c != '`' && c != '"' && c != '\\')) {
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
                if (prev_space)
                    continue;

                prev_space = true;
                tokens.emplace_back(current_token);
                current_token = {};
                continue;
            }
            current_token += c;
            prev_space = false;
        }
        if (!current_token.empty())
            tokens.emplace_back(current_token);

        return tokens;
    }

    bool is_delimeter(const std::string& name) {
        return name == "1>" || name == ">" || name == ">>" || name == "1>>" || name == "2>" || name == "2>>" ||
               name == "|" || name == "&|";
    }

} // namespace shell