#include "ast.h"

namespace parser
{
boost::optional<ast::err_t> parse(const std::string &code, ast::expr_value &ast);

}  // namespace parser
