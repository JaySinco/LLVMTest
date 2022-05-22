#pragma once
#include "../utils.h"
#include "prec.h"
#include <glog/logging.h>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

namespace parser
{

template <typename Iterator>
struct error_handler
{
    template <typename, typename, typename, typename>
    struct result
    {
        typedef void type;
    };

    error_handler(const std::filesystem::path& source_file): source_file(source_file) {}

    void operator()(Iterator first, Iterator last, Iterator err_pos,
                    const boost::spirit::info& what) const
    {
        Iterator ln_start = boost::spirit::get_line_start(first, err_pos);
        Iterator ln_end = boost::spirit::get_line_end(err_pos, last);
        int ln_pos = std::distance(ln_start, err_pos);
        int line = boost::spirit::get_line(err_pos);
        LOG(ERROR) << "\n"
                   << utils::ws2s(source_file.generic_wstring()) << ":" << line << "\n"
                   << utils::ws2s(std::wstring(ln_start, ln_end)) << "\n"
                   << std::string(ln_pos, ' ') << "^\n\n"
                   << what << " expected";
    }

    std::filesystem::path source_file;
};

}  // namespace parser
