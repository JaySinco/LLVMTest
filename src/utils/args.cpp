#include "args.h"
#include "logging.h"
#include <filesystem>

namespace utils
{

Args::Args(): opt_args_("Optional arguments"), opt_args_init_(opt_args_.add_options()) {}

void Args::optional(char const* name, po::value_semantic const* s, char const* description)
{
    opt_args_init_(name, s, description);
}

void Args::positional(char const* name, int max_count) { pos_args_.add(name, max_count); }

void Args::addSub(std::string const& name, std::string const& desc)
{
    subs_[name] = std::make_pair(std::make_shared<Args>(), desc);
}

void Args::parse(int argc, char const* const argv[], bool init_logger)
{
    po::command_line_parser parser(argc, argv);
    if (hasOptional()) {
        parser.options(opt_args_);
    }
    if (hasPositional()) {
        parser.positional(pos_args_);
    }
    po::store(parser.run(), vars_);
}

void Args::parse(std::vector<std::string> const& args) {}

void Args::printUsage(std::string& program, std::ostream& os)
{
    os << "Usage: " << program;
    if (hasOptional()) {
        os << " [options]";
    }
    if (hasPositional()) {
        std::string last = "";
        for (int i = 0; i < pos_args_.max_total_count(); ++i) {
            auto& name = pos_args_.name_for_position(i);
            if (name == last) {
                os << " ...";
                break;
            }
            last = name;
            os << " " << name;
        }
    }
    if (hasOptional()) {
        os << std::endl << std::endl;
        os << opt_args_ << std::endl;
    }
    if (hasSub()) {
        os << std::endl << std::endl;
        os << "Sub commands:" << std::endl;
        for (auto& [k, v]: subs_) {
            os << "  " << k << "    " << v.second;
        }
    }
    os << std::endl;
}

bool Args::has(std::string const& name) const { return vars_.count(name) != 0; }

Args const& Args::getSub(std::string const& name) const { return *subs_.at(name).first; }

bool Args::hasOptional() const { return !opt_args_.options().empty(); }

bool Args::hasPositional() const { return pos_args_.max_total_count() > 0; }

bool Args::hasSub() const { return !subs_.empty(); }

void Args::addFlagHelp() { optional("help,h", po::bool_switch(), "shows help message and exits"); }

void Args::addFlagLogging()
{
    optional("logdir", po::value<std::string>()->default_value(ws2s(getExeDir() + L"/logs")),
             "log files directory");
    optional("logtostderr", po::value<bool>()->default_value(true), "log messages go to stderr");
    optional("minloglevel", po::value<int>()->default_value(kDEBUG), "log level (0-6)");
    optional("stderrlevel", po::value<int>()->default_value(kINFO), "stderr log level");
    optional("logbuflevel", po::value<int>()->default_value(kERROR), "log buffered level");
    optional("logbufsecs", po::value<int>()->default_value(30), "max secs logs may be buffered");
    optional("maxlogsize", po::value<int>()->default_value(100), "max log file size (MB)");
}

void Args::addFlagSubcommand() {}

}  // namespace utils
