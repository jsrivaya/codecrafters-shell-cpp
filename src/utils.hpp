#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <set>

#if defined(__APPLE__) && defined(__MACH__)
#include <libproc.h>
#include <sys/proc_info.h>
#endif   

#include <string>
#include <map>

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

#if defined(__APPLE__) && defined(__MACH__)

std::map <int, std::string> fdtype_to_string {
    { PROX_FDTYPE_ATALK, "PROX_FDTYPE_ATALK" },      // (AppleTalk)
    { PROX_FDTYPE_VNODE, "PROX_FDTYPE_VNODE"},       // (Files)
    { PROX_FDTYPE_SOCKET, "PROX_FDTYPE_SOCKET"},     // (Sockets)
    { PROX_FDTYPE_PSHM, "PROX_FDTYPE_PSHM"},         // (POSIX Shared Memory)
    { PROX_FDTYPE_PSEM, "PROX_FDTYPE_PSEM"},         // (POSIX Semaphores)
    { PROX_FDTYPE_KQUEUE, "PROX_FDTYPE_KQUEUE"},     // (kqueue)
    { PROX_FDTYPE_PIPE, "PROX_FDTYPE_PIPE"},         // (Pipes)
    { PROX_FDTYPE_FSEVENTS, "PROX_FDTYPE_FSEVENTS"}, // (File System Events)
    { PROX_FDTYPE_NETPOLICY, "PROX_FDTYPE_NETPOLICY"} // (Network Policy)
};

void list_fds(pid_t pid) {
    // Get buffer size needed
    int bufsize = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, NULL, 0);
    if (bufsize <= 0) return;

    // Allocate buffer
    auto fds = std::make_unique<proc_fdinfo[]>(bufsize / PROC_PIDLISTFD_SIZE);

    // Get file descriptor info
    int n = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, fds.get(), bufsize);
    if (n <= 0) return;

    n /= PROC_PIDLISTFD_SIZE;

    // Print each fd
    for (int i = 0; i < n; ++i) {
        std::cerr << "FD " << fds[i].proc_fd << " type: " << fdtype_to_string[fds[i].proc_fdtype] << std::endl;
    }
}
#endif // defined(__APPLE__) && defined(__MACH__)

} // namespace shell
