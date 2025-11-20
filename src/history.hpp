#pragma once

#include "logger.hpp"

#include <algorithm>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <ranges>
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
    void append_history_to_file(const std::string& history_file = "") {  
        std::lock_guard<std::mutex> lock(mtx);
        auto path = history_file.empty() ? get_path() : history_file;   

        // load file in memory
        std::deque<std::string> file_content{};
        std::string line{};
        std::ifstream file_read(path);
        while (file_read.is_open() && std::getline(file_read, line)) {
            // shell::Logger::getInstance().log("read from file: " + line);
            file_content.emplace_back(line);
        }

        int itr_file = file_content.size() > 0 ?  file_content.size() - 1 : 0; // we start reading from the end of file
        int itr_cache = 0; // we start reading from the begining of cache
        int itr_to_append = 0;
        // looking for the longest cache list that matches file history EOF
        while(!file_content.empty() && itr_file >=0) {
            if(cached_history.at(itr_cache) == file_content.at(itr_file)) {
                auto itr_file_tmp = itr_file;
                auto itr_cache_tmp = itr_cache;

                while(itr_file_tmp != file_content.size() &&
                    file_content.at(itr_file_tmp) == cached_history.at(itr_cache_tmp)) {
                        ++itr_cache_tmp;
                        ++itr_file_tmp;
                }
                if (itr_file_tmp == file_content.size()) {
                    itr_to_append = itr_cache_tmp;
                    break;
                }
                ++itr_cache;

            } else {
                --itr_file;
            }
        }

        std::ofstream append_file(path, std::ios::app);
        while (itr_to_append < cached_history.size()) {
            append_file << cached_history.at(itr_to_append) << std::endl;
            ++itr_to_append;
        }
        append_file.close();
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