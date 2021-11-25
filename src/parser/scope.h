#pragma once
#include "./type.h"

namespace scope
{
class Scope
{
public:
    virtual std::optional<type::TypePtr> getVar(const std::string &id) const = 0;
    virtual std::map<std::string, type::TypePtr> getAllVar() const = 0;
    virtual ~Scope(){};
};

class LinearScope: public Scope
{
public:
    std::optional<type::TypePtr> getVar(const std::string &id) const override
    {
        if (this->storage.find(id) == this->storage.end()) {
            return {};
        }
        return this->storage.at(id);
    }

    std::map<std::string, type::TypePtr> getAllVar() const override { return this->storage; }

private:
    std::map<std::string, type::TypePtr> storage;
};

}  // namespace scope
