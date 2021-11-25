#include "./expr.h"
#include <boost/algorithm/string.hpp>
#define CHECK_EXPR(expr) \
    if (!expr) return invalid_expr(expr);

#define CHECK_SUBEXPR(expr, sub) \
    auto sub = expr->sub();      \
    if (!sub) return invalid_expr(expr);

namespace expr
{
nonstd::expected_lite::unexpected_type<type::Error> invalid_expr(antlr4::ParserRuleContext *expr)
{
    auto start = expr->getStart();
    auto line = start->getLine();
    auto charPosInLine = start->getCharPositionInLine();
    type::Error err = {line, charPosInLine, fmt::format("invalid expr: {}", expr->getText())};
    return nonstd::make_unexpected(err);
}

nonstd::expected<Property, type::Error> infer(parser::parsers::LiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto lit = expr->literal();
    if (!lit) {
        return invalid_expr(expr);
    }
    if (lit->NullLiteral()) {
        return Property{type::null, false};
    } else if (lit->BooleanLiteral()) {
        return Property{type::boolean, false};
    } else if (lit->StringLiteral()) {
        return Property{type::string, false};
    } else if (lit->numericLiteral() || lit->bigintLiteral()) {
        return Property{type::number, false};
    } else {
        return invalid_expr(expr);
    }
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ArrayLiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto exprSeq = expr->expressionSequence();
    if (!exprSeq || exprSeq->singleExpression().size() <= 0) {
        auto type = std::make_shared<type::Array>();
        type->internal = type::any;
        return Property{type, false};
    }
    auto exprList = exprSeq->singleExpression();
    auto type0 = infer(exprList.at(0), scope);
    if (!type0) {
        return type0.get_unexpected();
    }
    type::TypePtr merged = type0->type;
    for (int i = 1; i < exprList.size(); ++i) {
        auto type = infer(exprList.at(i), scope);
        if (!type) {
            return type.get_unexpected();
        }
        if (!merged->isConvertible(*type->type)) {
            if (type->type->isConvertible(*merged)) {
                merged = type->type;
            } else {
                merged = type::any;
            }
        }
    }
    auto type = std::make_shared<type::Array>();
    type->internal = merged;
    return Property{type, false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ObjectLiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto objLit = expr->objectLiteral();
    if (!objLit) {
        return invalid_expr(expr);
    }
    std::map<std::string, type::TypePtr> fields;
    for (const auto &prop: objLit->propertyAssignment()) {
        CHECK_EXPR(prop);
        CHECK_SUBEXPR(prop, propertyName);
        std::string name = propertyName->getText();
        if (auto strLit = propertyName->StringLiteral()) {
            boost::trim_if(name, boost::is_any_of("\""));
        }
        auto type = infer(prop->singleExpression(), scope);
        if (!type) {
            return type.get_unexpected();
        }
        fields[name] = (*type).type;
    }
    auto type = std::make_shared<type::Struct>();
    type->fields = std::move(fields);
    return Property{type, false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::SingleExpressionContext *expr,
                                              const scope::Scope &scope)
{
    if (auto litExpr = dynamic_cast<parser::parsers::LiteralExpressionContext *>(expr)) {
        return infer(litExpr, scope);
    } else if (auto arrLitExpr =
                   dynamic_cast<parser::parsers::ArrayLiteralExpressionContext *>(expr)) {
        return infer(arrLitExpr, scope);
    } else if (auto objLitExpr =
                   dynamic_cast<parser::parsers::ObjectLiteralExpressionContext *>(expr)) {
        return infer(objLitExpr, scope);
    } else {
        return invalid_expr(expr);
    }
}

}  // namespace expr
