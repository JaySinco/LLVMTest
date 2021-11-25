#include "./type.h"
#include <sstream>
#include <boost/algorithm/string.hpp>
#define INVALID_EXPR(expr) \
    nonstd::make_unexpected(fmt::format("invalid expr: {}", expr->getText()));

#define CHECK_EXPR(expr) \
    if (!expr) return INVALID_EXPR(expr);

#define CHECK_SUBEXPR(expr, sub) \
    auto sub = expr->sub();      \
    if (!sub) return INVALID_EXPR(expr);

namespace type
{
std::shared_ptr<Any> any = std::make_shared<Any>();
std::shared_ptr<Null> null = std::make_shared<Null>();
std::shared_ptr<Number> number = std::make_shared<Number>();
std::shared_ptr<Boolean> boolean = std::make_shared<Boolean>();
std::shared_ptr<String> string = std::make_shared<String>();

std::string Array::toString() { return fmt::format("array<{}>", this->internal->toString()); }

std::string Object::toString() { return fmt::format("object<{}>", this->internal->toString()); }

std::string Struct::toString()
{
    std::ostringstream ss;
    ss << "struct " << this->name << "{ ";
    for (const auto &[k, v]: this->internal) {
        ss << k << ": " << v->toString() << "; ";
    }
    ss << "}";
    return ss.str();
}

nonstd::expected<std::shared_ptr<Type>, std::string> infer(
    parser::parsers::LiteralExpressionContext *expr)
{
    auto lit = expr->literal();
    if (!lit) {
        return INVALID_EXPR(expr);
    }
    if (lit->NullLiteral()) {
        return null;
    } else if (lit->BooleanLiteral()) {
        return boolean;
    } else if (lit->StringLiteral()) {
        return string;
    } else if (lit->numericLiteral() || lit->bigintLiteral()) {
        return number;
    } else {
        return INVALID_EXPR(expr);
    }
}

nonstd::expected<std::shared_ptr<Type>, std::string> infer(
    parser::parsers::ObjectLiteralExpressionContext *expr)
{
    auto objLit = expr->objectLiteral();
    if (!objLit) {
        return INVALID_EXPR(expr);
    }
    std::map<std::string, std::shared_ptr<Type>> internalType;
    for (const auto &prop: objLit->propertyAssignment()) {
        CHECK_EXPR(prop);
        CHECK_SUBEXPR(prop, propertyName);
        std::string name = propertyName->getText();
        if (auto strLit = propertyName->StringLiteral()) {
            boost::trim_if(name, boost::is_any_of("\""));
        }
        auto type = infer(prop->singleExpression());
        if (!type) {
            return type.get_unexpected();
        }
        internalType[name] = *type;
    }
    auto type = std::make_shared<Struct>();
    type->internal = std::move(internalType);
    return type;
}

nonstd::expected<std::shared_ptr<Type>, std::string> infer(
    parser::parsers::SingleExpressionContext *expr)
{
    if (auto litExpr = dynamic_cast<parser::parsers::LiteralExpressionContext *>(expr)) {
        return infer(litExpr);
    } else if (auto objLitExpr =
                   dynamic_cast<parser::parsers::ObjectLiteralExpressionContext *>(expr)) {
        return infer(objLitExpr);
    } else {
        return INVALID_EXPR(expr);
    }
}

}  // namespace type
