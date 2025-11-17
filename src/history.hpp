#pragma once

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
    void persist_history() {
        std::lock_guard<std::mutex> lock(mtx);

        std::ofstream file(get_path());
        while(!cached_history.empty()) {
            file << (cached_history.front()) << std::endl;
            cached_history.pop_front();
        }
        file.close();
    }
    void print_last(unsigned number = 0) {
        if (cached_history.empty()) {
            std::cerr << "[print_last] History is empty" << std::endl;
            return;
        }
        if(number == 0)
            number = cached_history.size() - 1;

        int original_index = cached_history.size() - number;

        for (auto it = cached_history.rbegin(); it != cached_history.rend() && number > 0; ++it, ++original_index, --number) {
            std::cout << original_index << " " << it->data() << std::endl;
        }
    }
protected:
    std::string get_path() {
        auto home = std::getenv("HOME");
        std::string history_path = home ? home : "~";
        return history_path + "/" + name;
    }
    void load_history_from_file() {
        std::ifstream file(get_path());
        if (!file) return;

        std::string line;
        while (std::getline(file, line)) {
            cached_history.emplace_back(line);
        }
    }
private:
    History() { load_history_from_file(); };
    History(const History&) = delete;
    History& operator=(const History&) = delete;

    std::deque<std::string> cached_history;
    std::string name = ".shell_history";

    std::mutex mtx;
};

} // namespace shell