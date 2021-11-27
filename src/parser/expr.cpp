#include "./expr.h"
#include <boost/algorithm/string.hpp>

#define CHECK_EXPR(expr) \
    if (!expr) return makeError(expr, "invalid expr");

#define CHECK_SUBEXPR(expr, sub) \
    auto sub = expr->sub();      \
    if (!sub) return makeError(expr, "invalid expr");

#define CHECK_INFER(type, expr, scope)             \
    auto type##_ = infer(expr, scope);             \
    if (!type##_) return type##_.get_unexpected(); \
    auto &type = *type##_;

namespace expr
{
std::string evalStrLiteral(const std::string &s)
{
    // TODO: handle escape
    return s.substr(1, s.size() - 2);
}

nonstd::expected_lite::unexpected_type<type::Error> makeError(antlr4::ParserRuleContext *expr,
                                                              const std::string &desc)
{
    auto start = expr->getStart();
    auto line = start->getLine();
    auto charPosInLine = start->getCharPositionInLine();
    type::Error err = {line, charPosInLine, fmt::format("{}: {}", desc, expr->getText())};
    return nonstd::make_unexpected(err);
}

nonstd::expected_lite::unexpected_type<type::Error> makeError(antlr4::tree::TerminalNode *node,
                                                              const std::string &desc)
{
    auto start = node->getSymbol();
    auto line = start->getLine();
    auto charPosInLine = start->getCharPositionInLine();
    type::Error err = {line, charPosInLine, desc};
    return nonstd::make_unexpected(err);
}

nonstd::expected<Property, type::Error> infer(parser::parsers::MemberIndexExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto expr0 = expr->singleExpression(0);
    CHECK_EXPR(expr0);
    CHECK_INFER(type0, expr0, scope);
    auto expr1 = expr->singleExpression(1);
    CHECK_EXPR(expr1);
    CHECK_INFER(type1, expr1, scope);
    if (auto str = dynamic_cast<type::String *>(type0.type.get())) {
        if (!dynamic_cast<type::Number *>(type1.type.get())) {
            return makeError(expr1, fmt::format("{} can't member index string, number expected",
                                                type1.type->toString()));
        }
        return Property{type::string, type0.lvalue};
    } else if (auto arr = dynamic_cast<type::Array *>(type0.type.get())) {
        if (!dynamic_cast<type::Number *>(type1.type.get())) {
            return makeError(expr1, fmt::format("{} can't member index {}, number expected",
                                                type1.type->toString(), type0.type->toString()));
        }
        return Property{arr->internal, type0.lvalue};
    } else if (auto obj = dynamic_cast<type::Object *>(type0.type.get())) {
        if (!dynamic_cast<type::String *>(type1.type.get())) {
            return makeError(expr1, fmt::format("{} can't member index {}, string expected",
                                                type1.type->toString(), type0.type->toString()));
        }
        return Property{obj->internal, type0.lvalue};
    } else {
        return makeError(expr->OpenBracket(),
                         fmt::format("{} don't support member index", type0.type->toString()));
    }
}

nonstd::expected<Property, type::Error> infer(parser::parsers::MemberDotExpressionContext *expr,
                                              const scope::Scope &scope)
{
    CHECK_SUBEXPR(expr, singleExpression);
    CHECK_INFER(type, singleExpression, scope);
    if (auto stru = dynamic_cast<type::Struct *>(type.type.get())) {
        CHECK_SUBEXPR(expr, identifierName);
        std::string member = identifierName->getText();
        auto it = stru->fields.find(member);
        if (it == stru->fields.end()) {
            return makeError(identifierName, "member not exist");
        }
        return Property{it->second, type.lvalue};
    } else {
        return makeError(expr->Dot(),
                         fmt::format("{} don't support member dot", type.type->toString()));
    }
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ArgumentsExpressionContext *expr,
                                              const scope::Scope &scope)
{
    CHECK_SUBEXPR(expr, singleExpression);
    CHECK_INFER(type, singleExpression, scope);
    if (auto func = dynamic_cast<type::Function *>(type.type.get())) {
        std::vector<parser::parsers::SingleExpressionContext *> exprList;
        if (auto exprSeq = expr->expressionSequence()) {
            exprList = exprSeq->singleExpression();
        }
        if (exprList.size() != func->args.size()) {
            return makeError(expr->OpenParen(),
                             fmt::format("wrong arg number, {} expected", func->args.size()));
        }
        for (int i = 0; i < exprList.size(); ++i) {
            CHECK_INFER(arg, exprList.at(i), scope);
            if (!func->args.at(i)->isConvertible(*arg.type)) {
                return makeError(exprList.at(i), fmt::format("wrong arg type, {} expected",
                                                             func->args.at(i)->toString()));
            }
        }
        return Property{func->ret, false};
    } else {
        return makeError(expr->OpenParen(),
                         fmt::format("{} don't support function call", type.type->toString()));
    }
}

nonstd::expected<Property, type::Error> infer(parser::parsers::IdentifierExpressionContext *expr,
                                              const scope::Scope &scope)
{
    if (auto type = scope.getVar(expr->getText())) {
        return Property{*type, true};
    }
    return makeError(expr, "identifier not found");
}

nonstd::expected<Property, type::Error> infer(parser::parsers::LiteralExpressionContext *expr,
                                              const scope::Scope &scope)
{
    auto lit = expr->literal();
    CHECK_EXPR(lit);
    if (lit->NullLiteral()) {
        return Property{type::null, false};
    } else if (lit->BooleanLiteral()) {
        return Property{type::boolean, false};
    } else if (lit->StringLiteral()) {
        return Property{type::string, false};
    } else if (lit->numericLiteral() || lit->bigintLiteral()) {
        return Property{type::number, false};
    } else {
        return makeError(expr, "unknown literal");
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
    CHECK_INFER(type0, exprList.at(0), scope);
    type::TypePtr merged = type0.type;
    for (int i = 1; i < exprList.size(); ++i) {
        CHECK_INFER(type, exprList.at(i), scope);
        if (!merged->isConvertible(*type.type)) {
            if (type.type->isConvertible(*merged)) {
                merged = type.type;
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
    CHECK_EXPR(objLit);
    std::map<std::string, type::TypePtr> fields;
    for (const auto &prop: objLit->propertyAssignment()) {
        CHECK_EXPR(prop);
        CHECK_SUBEXPR(prop, propertyName);
        std::string name = propertyName->getText();
        if (propertyName->StringLiteral()) {
            name = evalStrLiteral(name);
        }
        auto expr = prop->singleExpression();
        CHECK_EXPR(expr);
        CHECK_INFER(type, expr, scope);
        fields[name] = type.type;
    }
    return Property{std::make_shared<type::Struct>(std::move(fields)), false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::ParenthesizedExpressionContext *expr,
                                              const scope::Scope &scope)
{
    CHECK_SUBEXPR(expr, singleExpression);
    CHECK_INFER(type, singleExpression, scope);
    return Property{type.type, false};
}

nonstd::expected<Property, type::Error> infer(parser::parsers::SingleExpressionContext *expr,
                                              const scope::Scope &scope)
{
    if (auto memberIdxExpr = dynamic_cast<parser::parsers::MemberIndexExpressionContext *>(expr)) {
        return infer(memberIdxExpr, scope);
    } else if (auto memberDotExpr =
                   dynamic_cast<parser::parsers::MemberDotExpressionContext *>(expr)) {
        return infer(memberDotExpr, scope);
    } else if (auto argsExpr = dynamic_cast<parser::parsers::ArgumentsExpressionContext *>(expr)) {
        return infer(argsExpr, scope);
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
        return makeError(expr, "unknown expr");
    }
}

}  // namespace expr
