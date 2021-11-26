#include "./expr.h"
#include <boost/algorithm/string.hpp>

#define CHECK_EXPR(expr) \
    if (!expr) return make_error(expr, "invalid expr");

#define CHECK_SUBEXPR(expr, sub) \
    auto sub = expr->sub();      \
    if (!sub) return make_error(expr, "invalid expr");

namespace expr
{
nonstd::expected_lite::unexpected_type<type::Error> make_error(antlr4::ParserRuleContext *expr,
                                                               const std::string &desc)
{
    auto start = expr->getStart();
    auto line = start->getLine();
    auto charPosInLine = start->getCharPositionInLine();
    type::Error err = {line, charPosInLine, fmt::format("{}: {}", desc, expr->getText())};
    return nonstd::make_unexpected(err);
}

nonstd::expected_lite::unexpected_type<type::Error> make_error(antlr4::tree::TerminalNode *node,
                                                               const std::string &desc)
{
    auto start = node->getSymbol();
    auto line = start->getLine();
    auto charPosInLine = start->getCharPositionInLine();
    type::Error err = {line, charPosInLine, desc};
    return nonstd::make_unexpected(err);
}

nonstd::expected<Property, type::Error> infer(parser::parsers::MemberDotExpressionContext *expr,
                                              const scope::Scope &scope)
{
    CHECK_SUBEXPR(expr, singleExpression);
    auto type = infer(singleExpression, scope);
    if (!type) {
        return type.get_unexpected();
    }
    if (auto stru = dynamic_cast<type::Struct *>(type->type.get())) {
        CHECK_SUBEXPR(expr, identifierName);
        std::string member = identifierName->getText();
        auto it = stru->fields.find(member);
        if (it == stru->fields.end()) {
            return make_error(identifierName, "member not exist");
        }
        return Property{it->second, type->lvalue};
    } else {
        return make_error(
            expr->Dot(), fmt::format("type '{}' don't support member dot", type->type->toString()));
    }
}

nonstd::expected<Property, type::Error> infer(parser::parsers::IdentifierExpressionContext *expr,
                                              const scope::Scope &scope)
{
    if (auto type = scope.getVar(expr->getText())) {
        return Property{*type, true};
    }
    return make_error(expr, "identifier not found");
}

nonstd::expected<Property, type::Error> infer(parser::parsers::LiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto lit = expr->literal();
    if (!lit) {
        return make_error(expr, "invalid expr");
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
        return make_error(expr, "unknown literal");
    }
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ArrayLiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto exprSeq = expr->expressionSequence();
    if (!exprSeq || exprSeq->singleExpression().size() <= 0) {
        return Property{std::make_shared<type::Array>(type::any), false};
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
    return Property{std::make_shared<type::Array>(merged), false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ObjectLiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto objLit = expr->objectLiteral();
    if (!objLit) {
        return make_error(expr, "invalid expr");
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
    return Property{std::make_shared<type::Struct>(std::move(fields)), false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ParenthesizedExpressionContext *expr,
                                              const scope::Scope &scope)
{
    CHECK_SUBEXPR(expr, singleExpression);
    auto type = infer(singleExpression, scope);
    if (!type) {
        return type.get_unexpected();
    }
    return Property{(*type).type, false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::SingleExpressionContext *expr,
                                              const scope::Scope &scope)
{
    if (auto memberDotExpr = dynamic_cast<parser::parsers::MemberDotExpressionContext *>(expr)) {
        return infer(memberDotExpr, scope);
    } else if (auto idExpr = dynamic_cast<parser::parsers::IdentifierExpressionContext *>(expr)) {
        return infer(idExpr, scope);
    } else if (auto litExpr = dynamic_cast<parser::parsers::LiteralExpressionContext *>(expr)) {
        return infer(litExpr, scope);
    } else if (auto arrLitExpr =
                   dynamic_cast<parser::parsers::ArrayLiteralExpressionContext *>(expr)) {
        return infer(arrLitExpr, scope);
    } else if (auto objLitExpr =
                   dynamic_cast<parser::parsers::ObjectLiteralExpressionContext *>(expr)) {
        return infer(objLitExpr, scope);
    } else if (auto parenExpr =
                   dynamic_cast<parser::parsers::ParenthesizedExpressionContext *>(expr)) {
        return infer(parenExpr, scope);
    } else {
        return make_error(expr, "unknown expr");
    }
}

}  // namespace expr
