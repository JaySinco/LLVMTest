#pragma once
#include <boost/program_options.hpp>
#include <iostream>

namespace utils
{

namespace po = boost::program_options;

class Args
{
public:
    Args();
    void optional(char const* name, po::value_semantic const* s, char const* description);
    void positional(char const* name, int max_count);
    void addSub(std::string const& name, std::string const& desc);
    void parse(int argc, char const* const argv[], bool init_logger = true);
    void parse(std::vector<std::string> const& args);
    void printUsage(std::string& program, std::ostream& os = std::cerr);
    bool has(std::string const& name) const;
    Args const& getSub(std::string const& name) const;

    template <typename T>
    T const& get(std::string const& name) const
    {
        return vars_[name].as<T>();
    }

private:
    bool hasOptional() const;
    bool hasPositional() const;
    bool hasSub() const;
    void addFlagHelp();
    void addFlagLogging();
    void addFlagSubcommand();

    po::options_description opt_args_;
    po::options_description_easy_init opt_args_init_;
    po::positional_options_description pos_args_;
    po::variables_map vars_;
    std::map<std::string, std::pair<std::shared_ptr<Args>, std::string>> subs_;
};

}  // namespace utils
