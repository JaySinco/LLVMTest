#pragma once
#include <string>
#include <vector>
#include <optional>

namespace ast
{
using quoted = std::wstring;
using column = std::wstring;
using line = std::vector<column>;

}  // namespace ast

struct error
{
    std::string message;
};

std::optional<error> parse(const std::wstring &raw, ast::line &ast);
