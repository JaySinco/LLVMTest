#include "prec.h"
#include "utils/base.h"
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <fmt/ranges.h>
#include <fmt/ostream.h>

namespace qi = boost::spirit::qi;
namespace enc = boost::spirit::standard_wide;
namespace phx = boost::phoenix;

namespace ast
{
struct employee
{
    int age;
    std::wstring surname;
    std::wstring forename;
    double salary;
};

}  // namespace ast

BOOST_FUSION_ADAPT_STRUCT(ast::employee,
                          (int, age)(std::wstring, surname)(std::wstring, forename)(double, salary))

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
        spdlog::error("{}({},{}): error: {} expected\n{}\n{}^",
                      utils::ws2s(source_file.filename().generic_wstring()), line, ln_pos + 1, what,
                      utils::ws2s(std::wstring(ln_start, ln_end)), std::string(ln_pos, ' '));
    }

    std::filesystem::path source_file;
};

template <typename Iterator>
struct expression: qi::grammar<Iterator, ast::employee()>
{
    expression(const std::filesystem::path& source_file)
        : expression::base_type(start), err_handler(source_file)
    {
        quoted = qi::lexeme['"' > *(enc::char_ - '"') > '"'];
        epl = qi::lit(L"employee") > '{' > qi::int_ > ',' > quoted > ',' > quoted > ',' >
              qi::double_ > '}';
        start = qi::skip(enc::space)[qi::eps > epl > qi::eoi];

        qi::on_error<qi::fail>(start, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
    }

    qi::rule<Iterator, std::wstring()> quoted;
    qi::rule<Iterator, ast::employee()> epl;
    qi::rule<Iterator, ast::employee()> start;

    phx::function<error_handler<Iterator>> err_handler;
};

}  // namespace parser

void parsing(const std::filesystem::path& source_file)
{
    using iterator = boost::spirit::line_pos_iterator<std::wstring::const_iterator>;
    auto raw = utils::readFile(source_file.wstring());
    std::wstring input = utils::s2ws(*raw, true);
    iterator beg(input.begin());
    iterator end(input.end());
    parser::expression<iterator> expr(source_file);
    ast::employee attr;
    bool ok = qi::parse(beg, end, expr, attr);
    spdlog::info("{} {}", ok, utils::ws2s(attr.surname));
}

int main(int argc, char** argv) { parsing(__DIRNAME__ / "input.txt"); }
