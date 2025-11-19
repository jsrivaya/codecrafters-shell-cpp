#pragma once

#include "logger.hpp"

#include <algorithm>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>

namespace shell {

class History {
public:
    static History& getInstance() {
        static History instance;
        return instance;
    }
    void log_command_line(const std::string& command) {
        std::lock_guard<std::mutex> lock(mtx);

        cached_history.emplace_back(command);
    }
    void persist_history(const std::string& history_file = "") {
        std::lock_guard<std::mutex> lock(mtx);
        std::ofstream file;

        if (history_file.empty())
            file = std::ofstream(get_path());
        else
            file =std::ofstream(history_file);

        for (const auto& cmd : cached_history) {
            file << cmd << std::endl;
        }

        file.close();
    }
    void print_last(unsigned number = 0) {
        if (cached_history.empty()) {
            return;
        }
        if(number == 0)
            number = cached_history.size();

        int original_index = cached_history.size() - number + 1;

        for (auto it = cached_history.end() - number; it != cached_history.end() && number > 0; ++it, ++original_index, --number) {
            std::cout << "    " << original_index << "  " << it->data() << std::endl;
        }
    }
    void load_history_from_file(const std::string& requested_file = "") {

        std::ifstream file;
        if(requested_file.empty() && cached_history.empty()) {
            file = std::ifstream(persistent_path);
        } else if (!requested_file.empty()) {
            file = std::ifstream(requested_file);
        }
        if (!file) return;

        std::string line;
        while (std::getline(file, line)) {
            if(!line.empty()) cached_history.emplace_back(line);
        }
    }

protected:
    std::string get_path() {
        return persistent_path;
    }

private:
    History() {
        if (const char* env = std::getenv("HISTFILE")) {
            persistent_path = std::string{env};
        } else {
            persistent_path = "~/.shell_history";
        }
        load_history_from_file();
    };
    History(const History&) = delete;
    History& operator=(const History&) = delete;

    std::deque<std::string> cached_history;
    std::string persistent_path;

    std::mutex mtx;
};

} // namespace shell