#include "command.hpp"
#include "command_builtin.hpp"
#include "command_custom.hpp"

namespace shell {

    std::unique_ptr<Command> Command::create(const std::string& name) {
        if (BuiltinCommand::is_builtin(name))
            return std::make_unique<BuiltinCommand>(name);
        return std::make_unique<CustomCommand>(name);
    }

} // namespace shell 