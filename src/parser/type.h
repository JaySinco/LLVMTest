#pragma once
#include "../utils.h"
#include ".antlr/lexers.h"
#include ".antlr/parsers.h"

namespace type
{
class Type
{
public:
    virtual std::string toString() = 0;
    virtual ~Type(){};
};

nonstd::expected<std::shared_ptr<Type>, std::string> infer(
    parser::parsers::SingleExpressionContext *expr);

struct Any: Type
{
    std::string toString() override { return "any"; }
};

struct Null: Type
{
    std::string toString() override { return "null"; }
};

struct Number: Type
{
    std::string toString() override { return "number"; }
};

struct Boolean: Type
{
    std::string toString() override { return "boolean"; }
};

struct String: Type
{
    std::string toString() override { return "string"; }
};

extern std::shared_ptr<Any> any;
extern std::shared_ptr<Null> null;
extern std::shared_ptr<Number> number;
extern std::shared_ptr<Boolean> boolean;
extern std::shared_ptr<String> string;

struct Array: Type
{
    std::string toString() override;
    std::shared_ptr<Type> internal;
};

struct Object: Type
{
    std::string toString() override;
    std::shared_ptr<Type> internal;
};

struct Struct: public Type
{
    std::string toString() override;
    std::string name;
    std::map<std::string, std::shared_ptr<Type>> internal;
};

}  // namespace type
