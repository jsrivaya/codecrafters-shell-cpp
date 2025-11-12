#include "command.hpp"
#include "command_builtin.hpp"
#include "command_custom.hpp"

#include <memory>

namespace shell {

    std::shared_ptr<Command> Command::get_command(const std::string& name) {

        if (BuiltinCommand::is_builtin(name)) {
            return std::make_shared<BuiltinCommand>(name);
        }

        return std::make_shared<CustomCommand>(name);
    }
} // namespace shell 