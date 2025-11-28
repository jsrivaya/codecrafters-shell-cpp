#include "command.hpp"
#include "command_builtin.hpp"
#include "command_custom.hpp"

#include <memory>

namespace shell {

    std::shared_ptr<Command> Command::get_command(const std::vector<std::string>& args) {
        auto name = args.at(0);
        auto rest_args = std::vector<std::string>(std::next(args.begin()), args.end());

        if (BuiltinCommand::is_builtin(name)) {
            return std::make_shared<BuiltinCommand>(name, rest_args);
        }

        return std::make_shared<CustomCommand>(name, rest_args);
    }

} // namespace shell