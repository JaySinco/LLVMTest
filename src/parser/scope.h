#pragma once
#include "./type.h"

namespace scope
{
class Scope
{
public:
    virtual std::optional<type::TypePtr> getVar(const std::string &id) const = 0;
    virtual std::optional<type::TypePtr> getTypeAlias(const std::string &id) const = 0;
    virtual std::map<std::string, type::TypePtr> getAllVar() const = 0;
    virtual std::map<std::string, type::TypePtr> getAllTypeAlias() const = 0;
    virtual ~Scope(){};
};

class LinearScope: public Scope
{
public:
    LinearScope()
    {
        vars.emplace("z", type::any);
        vars.emplace("b", type::boolean);
        vars.emplace("n", type::number);
        vars.emplace("s", type::string);
        vars.emplace("arr", std::make_shared<type::Array>(type::number));
        vars.emplace("obj", std::make_shared<type::Object>(type::string));
        vars.emplace("m", std::make_shared<type::Struct>(std::map<std::string, type::TypePtr>{
                              {"age", type::number},
                              {"succ", type::boolean},
                              {"address", type::string},
                          }));
        vars.emplace("fn",
                     std::make_shared<type::Function>(
                         std::vector<type::TypePtr>{type::string, type::number}, type::string));
    }

    std::optional<type::TypePtr> getVar(const std::string &id) const override
    {
        if (this->vars.find(id) == this->vars.end()) {
            return {};
        }
        return this->vars.at(id);
    }

    std::optional<type::TypePtr> getTypeAlias(const std::string &id) const override
    {
        if (this->typeAliases.find(id) == this->typeAliases.end()) {
            return {};
        }
        return this->typeAliases.at(id);
    }

    std::map<std::string, type::TypePtr> getAllVar() const override { return this->vars; }

    std::map<std::string, type::TypePtr> getAllTypeAlias() const override
    {
        return this->typeAliases;
    }

private:
    std::map<std::string, type::TypePtr> vars;
    std::map<std::string, type::TypePtr> typeAliases;
};

}  // namespace scope
