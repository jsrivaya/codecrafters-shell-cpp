#include "command.hpp"
#include "command_builtin.hpp"
#include "command_custom.hpp"

#include <mutex>
#include <unordered_map>

namespace shell {

    std::shared_ptr<Command> Command::get(const std::string& name) {
        static std::unordered_map<std::string, std::shared_ptr<Command>> cache;

        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        if (BuiltinCommand::is_builtin(name)) {
            if(cache.contains(name)) {
                return cache.at(name);
            }
            cache.emplace(name, std::make_shared<BuiltinCommand>(name));
            return cache.at(name);
        }

        return std::make_shared<CustomCommand>(name);
    }
} // namespace shell 