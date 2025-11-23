#pragma once

#include <filesystem>
#include <unistd.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <iostream>
#include <libproc.h>
#include <map>
#include <string>
#include <sys/proc_info.h>
#endif   

namespace shell {

static void reset_stdio(int io_fileno, int saved_stdio) {
    dup2(saved_stdio, io_fileno); // Restore the saved fd to stdout
    close(saved_stdio); // Clean up
}

static bool is_executable(const std::filesystem::path& p) {
    return std::filesystem::is_regular_file(p) && 
           access(p.c_str(), X_OK) == 0;
}

#if defined(__APPLE__) && defined(__MACH__)

static std::map <int, std::string> fdtype_to_string {
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

static void list_fds(pid_t pid) {
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
