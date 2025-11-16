#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <set>

namespace shell {
    // custom file descriptor deleter
    // struct fd_deleter {
    //     void operator()(int fd) const { if (fd >= 0) close(fd); }
    // };
    // using unique_fd = std::unique_ptr<int, fd_deleter>;
    // auto fd_deleter = [](int fd) { if (fd >= 0) close(fd); };
    // using unique_fd = std::unique_ptr<int, decltype(fd_deleter)>;

    // class file_descriptor {
    //     int fd_{-1};
    // public:
    //     file_descriptor(int fd = -1): fd_(fd) {}
    //     file_descriptor(nullptr_t) {}
    //     operator int() const { return fd_; }
    //     explicit operator bool() const { return fd_ != -1; }
    //     friend bool operator==(file_descriptor, file_descriptor) = default; // Since C++20
    // };
static std::set<std::string> commands_store{};

class file_descriptor {
    int fd_ = -1;
public:
    file_descriptor() = default;
    file_descriptor(int fd) : fd_(fd) {};
    file_descriptor(std::nullptr_t) {}
    explicit operator bool() const { return fd_ != -1; }
    operator int() const { return fd_; }
    friend bool operator==(file_descriptor, file_descriptor) = default;
};

struct fd_closer {
    using pointer = file_descriptor; // ← Must be a type that can be compared to nullptr
    void operator()(pointer fd) const noexcept {
        if (fd) ::close(static_cast<int>(fd));
    }
};

// struct unix_file;
// // RAII type using opaque handle
using unique_fd = std::unique_ptr<file_descriptor, fd_closer>;

// // Factory function to avoid nullptr issues
unique_fd make_unique_fd(int fd) {
    return unique_fd{fd};
}
// unique_fd fd_out{std::move(fd), fd_deleter};

bool is_executable(const std::filesystem::path& p) {
    return std::filesystem::is_regular_file(p) && 
           access(p.c_str(), X_OK) == 0;
}

// void load_commands(const std::string& path = "PATH") {
//     if(const auto p = std::getenv(path.c_str()); p) {
//         std::string path_env{p};
//         for(size_t start = 0; start < path_env.size();) {
//             size_t end = path_env.find_first_of(":", start);
//             if (end == std::string::npos) end = path_env.size();

//             // std::filesystem::path p = path_env.substr(start, end - start);
//             for (const auto& entry : std::filesystem::directory_iterator(path_env.substr(start, end - start))) {
//                if(is_executable(entry)) {
//                 commands_store.emplace(entry);
//                }
//             }
//             start = end + 1;
//         }
//     }  
// }

// std::vector<std::string> find_matches(const std::string& pattern) {
//     std::vector<std::string> matches{};
//     auto it = std::copy_if(commands_store.begin(), commands_store.end(), std::back_inserter(matches),
//         [pattern](const std::string& s) {
//             return s.rfind(pattern, 0) == 0;
//     });
//     return matches;
// }

} // namespace shell