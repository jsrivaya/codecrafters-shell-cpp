#pragma once
#include <string>

namespace shell {
std::string trim(const std::string& s) { // sniff sniff, not great
    std::string trimmed{};
    bool in_quotes = false;
    bool in_double_quotes = false;
    bool prev_space = false;
    bool in_escaped = false;
    for(const auto& c : s) {
        if (in_escaped) {
            trimmed += c;
            in_escaped = !in_escaped;
            continue;
        }
        if (in_quotes && c != '\'') {
            trimmed += c;
            continue;
        }
        if (c == '\\') {
            in_escaped = true;
            continue;
        }
        if (c == '"') {
            in_double_quotes = ! in_double_quotes;
            in_quotes = false;
            prev_space = false;
            continue;
        }
        if (in_double_quotes) {
            trimmed += c;
            continue;
        }
        if (c == '\'') {
            in_quotes = !in_quotes;
            prev_space = false;
            continue;
        }
        if (c == ' ' && !in_quotes) {
            if(prev_space) continue;

            prev_space = true;
            trimmed += c;
            continue;
        }
        trimmed += c;
        prev_space = false;
    }
    return trimmed;
}
} // namespace shell