#pragma once
#include "../utils.h"
#include ".antlr/lexers.h"
#include ".antlr/parsers.h"
#include <optional>

namespace type
{
struct Error
{
    size_t line;           //  1..n
    size_t charPosInLine;  //  0..n-1
    std::string msg;
};

struct Type
{
    virtual std::string toString() const = 0;
    virtual bool isConvertible(const Type &rhs) const = 0;
    virtual ~Type(){};
};

using TypePtr = std::shared_ptr<type::Type>;

struct Any: Type
{
    std::string toString() const override { return "any"; }

    bool isConvertible(const Type &rhs) const override { return true; }
};

struct Null: Type
{
    std::string toString() const override { return "null"; }

    bool isConvertible(const Type &rhs) const override
    {
        return dynamic_cast<const Null *>(&rhs) != nullptr;
    }
};

struct Number: Type
{
    std::string toString() const override { return "number"; }

    bool isConvertible(const Type &rhs) const override
    {
        return dynamic_cast<const Number *>(&rhs) != nullptr;
    }
};

struct Boolean: Type
{
    std::string toString() const override { return "boolean"; }

    bool isConvertible(const Type &rhs) const override
    {
        return dynamic_cast<const Boolean *>(&rhs) != nullptr;
    }
};

struct String: Type
{
    std::string toString() const override { return "string"; }

    bool isConvertible(const Type &rhs) const override
    {
        return dynamic_cast<const String *>(&rhs) != nullptr;
    }
};

extern std::shared_ptr<Any> any;
extern std::shared_ptr<Null> null;
extern std::shared_ptr<Number> number;
extern std::shared_ptr<Boolean> boolean;
extern std::shared_ptr<String> string;

struct Array: Type
{
    Array(TypePtr internal): internal(internal) {}
    std::string toString() const override;
    bool isConvertible(const Type &rhs) const override;

    TypePtr internal;
};

struct Object: Type
{
    Object(TypePtr internal): internal(internal) {}
    std::string toString() const override;
    bool isConvertible(const Type &rhs) const override;

    TypePtr internal;
};

struct Struct: Type
{
    Struct(const std::map<std::string, TypePtr> &fields): fields(fields) {}
    Struct(std::map<std::string, TypePtr> &&fields): fields(std::move(fields)) {}
    std::string toString() const override;
    bool isConvertible(const Type &rhs) const override;

    std::map<std::string, TypePtr> fields;
};

struct Function: Type
{
    Function(const std::vector<TypePtr> &args, TypePtr ret): args(args), ret(ret) {}
    Function(std::vector<TypePtr> &&args, TypePtr ret): args(std::move(args)), ret(ret) {}
    std::string toString() const override;
    bool isConvertible(const Type &rhs) const override;

    std::vector<TypePtr> args;
    TypePtr ret;
};

}  // namespace type
