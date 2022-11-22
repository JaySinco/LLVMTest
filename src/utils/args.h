#pragma once
#include <boost/program_options.hpp>
#include <iostream>

namespace utils
{

void printUsage(std::string const& program,
                boost::program_options::options_description const* opt_args = nullptr,
                boost::program_options::positional_options_description const* pos_args = nullptr,
                std::ostream& os = std::cerr);

}  // namespace utils
