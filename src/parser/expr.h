#pragma once
#include "./type.h"
#include "./scope.h"

namespace expr
{
struct Property
{
    type::TypePtr type;
    bool lvalue;
};

nonstd::expected<Property, type::Error> infer(parser::parsers::SingleExpressionContext *expr,
                                              const scope::Scope &scope);

}  // namespace expr
