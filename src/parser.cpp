#include "parser.h"
#include "utils.h"

namespace parser
{
struct compare_op_: x3::symbols<ast::op_t>
{
    compare_op_()
    {
        // clang-format off
        add
            ("=" , ast::op_t::EQUAL)
            ("!=", ast::op_t::NOT_EQUAL)
            (">" , ast::op_t::GREATER)
            (">=", ast::op_t::GREATER_EQUAL)
            ("<" , ast::op_t::LESS)
            ("<=", ast::op_t::LESS_EQUAL)
        ;
        // clang-format on
    }

} compare_op;

const x3::rule<class expr_class, ast::expr_value> expr = "expr";
const x3::rule<class comp_class, ast::comp_value> comp = "comp";

const auto plain = x3::lexeme[+x3::char_(".0-9a-zA-Z")];
const auto quoted = x3::lexeme['"' >> *(x3::char_ - '"') >> '"'];
const auto selector = x3::lexeme[+x3::char_("-0-9a-zA-Z") % '.'];
const auto target = plain | quoted;
const auto comp_def = compare_op >> target;
const auto match = selector >> -comp;
const auto unit = match | '(' >> expr >> ')';
const auto not_ = -x3::char_('!') >> unit;
const auto and_ = not_ % x3::repeat(1, 2)[x3::lit('&')];
const auto or_ = and_ % x3::repeat(1, 2)[x3::lit('|')];
const auto expr_def = or_;

BOOST_SPIRIT_DEFINE(expr, comp);

boost::optional<ast::err_t> parse(const std::string &code, ast::expr_value &ast)
{
    auto it = code.begin();
    bool ok = x3::phrase_parse(it, code.end(), expr, x3::space, ast);
    if (!ok || it != code.end()) {
        return {{"failed to parse filter: unexpected token near '{}'"_format(*it)}};
    }
    return {};
}

}  // namespace parser
