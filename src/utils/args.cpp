#include "args.h"
#include <filesystem>

namespace utils
{

void printUsage(std::string const& program,
                boost::program_options::options_description const* opt_args,
                boost::program_options::positional_options_description const* pos_args,
                std::ostream& os)
{
    os << "Usage: " << std::filesystem::path(program).filename().string();
    if (pos_args != nullptr) {
        os << " [options]";
    }
    if (pos_args != nullptr) {
        std::string last = "";
        for (int i = 0; i < pos_args->max_total_count(); ++i) {
            auto& name = pos_args->name_for_position(i);
            if (name == last) {
                os << " ...";
                break;
            }
            last = name;
            os << " " << name;
        }
    }
    os << std::endl << std::endl;
    if (pos_args != nullptr) {
        os << *opt_args << std::endl;
    }
}

}  // namespace utils
