#pragma once
#include "command_builtin.hpp"

#include <dirent.h>
#include <string>
#include <readline/history.h>
#include <readline/readline.h>
#include <vector>

namespace shell {

/* A static variable for holding the line. */
static char * line_read = (char *) NULL;

std::vector<std::string> load_paths() {
    std::vector<std::string> directories{};

    if(const auto& p = std::getenv("PATH"); p) {
        std::string path_env{p};
        for(size_t start = 0; start < path_env.size();) {
            size_t end = path_env.find_first_of(":", start);
            if (end == std::string::npos) end = path_env.size();
            directories.emplace_back(path_env.substr(start, end - start));

            start = end + 1;
        }
    }
    return directories;
}

char * path_files_generator(const char *text, int state) {
    static std::vector<std::string> paths{};
    static size_t dir_index;
    static DIR *dir;
    static struct dirent *entry;

    if (state == 0) {
        // First call: Open directory and return the first match.
        if(paths = load_paths(); paths.empty()) return NULL;
        dir_index = 0;
        dir = opendir(paths.at(dir_index).c_str());
        while (!dir && dir_index < paths.size()) {
          ++dir_index;
          if (dir_index < paths.size()) {
            dir = opendir(paths.at(dir_index).c_str());
            if (!dir) continue; // Skip unreadable directory
          } else {
              return NULL;
          }
        }
    }

    // Find the next entry that matches
    while (dir && dir_index < paths.size()) {
        if (entry = readdir(dir); entry) {
            if (strncmp(entry->d_name, text, strlen(text)) == 0) {
                return strdup(entry->d_name); // Return the FIRST match found on this call
            }
        } else { // end of files in dir; load next dir
            if (dir) { closedir(dir); dir = NULL; }

            for (dir_index+=1; !dir && dir_index < paths.size(); ++dir_index) {
                dir = opendir(paths.at(dir_index).c_str());
                if (dir) break;
            }
        }
    }

    // No more matches
    if (dir) {
        closedir(dir);
        dir = NULL;
    }
    return NULL;
}

char ** path_files_completion(const char * text, int start, int end) {
    // Prevent Readline from doing default filename completion
    rl_attempted_completion_over = 1;

    // search in our builtin commands inventory first
    auto builtin_commands = shell::BuiltinCommand::get_all_commands();
    auto it = std::find_if(builtin_commands.begin(), builtin_commands.end(),
      [text](const std::string& s) {
            return s.rfind(std::string(text), 0) == 0;
    });
    if (it != builtin_commands.end()) {
      static std::string match;
      match = it->c_str();
      return rl_completion_matches(text, [](const char* text, int state) -> char* {
        if (state == 0) return strdup(match.c_str());

        return nullptr;
      });
    }

    // search in PATH if not a builtin
    return rl_completion_matches(text, path_files_generator);
}

// Read a string, and return a pointer to it. Returns NULL on EOF.
char * rl_gets () {
  if (line_read) {
      free (line_read);
      line_read = (char *)NULL;
    }

  // print prompt and read line from user
  line_read = readline ("$ ");
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}

std::string read_line() {
  if (line_read) {
    free (line_read);
    line_read = (char *) NULL;
  }

  return std::string(rl_gets ());
}

void rl_init() {
  rl_initialize();
  rl_attempted_completion_function = path_files_completion;   
}
} // namespace shell