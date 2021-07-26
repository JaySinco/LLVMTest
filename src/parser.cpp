#include "parser.h"
#include "utils.h"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

namespace parser
{
struct doubleQuote_: x3::standard_wide::symbols<wchar_t>
{
    doubleQuote_() { add(L"\"\"", L'\"'); }
} doubleQuote;

x3::rule<struct quoted_class, ast::quoted> quoted{"quoted"};
x3::rule<struct column_class, ast::column> column{"column"};
x3::rule<struct line_class, ast::line> line{"line"};

const std::wstring colSep = L",";
const auto quoted_def = x3::no_skip[L'"' > *(doubleQuote | ~x3::char_(L'"')) > L'"'];
const auto column_def = quoted | x3::no_skip[*(x3::char_ - colSep)];
const auto line_def = column % x3::no_skip[colSep];

BOOST_SPIRIT_DEFINE(line, quoted, column);

}  // namespace parser

std::optional<error> parse(const std::wstring &ws, ast::line &ast)
{
    std::wstring::const_iterator iter = ws.begin();
    std::wstring::const_iterator end = ws.end();
    try {
        bool ok = x3::phrase_parse(iter, end, parser::line, x3::standard_wide::space, ast);
        if (!ok || iter != ws.end()) {
            std::wstring pos;
            pos.push_back(*iter);
            return {{"unexpected '{}' after: {}"_format(
                utils::ws2s(pos), utils::ws2s(ws.substr(0, iter - ws.begin())))}};
        }
    } catch (const x3::expectation_failure<std::wstring::const_iterator> &e) {
        return {{"expect {} after: {}"_format(e.which(),
                                              utils::ws2s(ws.substr(0, e.where() - ws.begin())))}};
    }
    return {};
}
