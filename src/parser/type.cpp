#include "./type.h"
#include <sstream>

namespace type
{
std::shared_ptr<Any> any = std::make_shared<Any>();
std::shared_ptr<Null> null = std::make_shared<Null>();
std::shared_ptr<Number> number = std::make_shared<Number>();
std::shared_ptr<Boolean> boolean = std::make_shared<Boolean>();
std::shared_ptr<String> string = std::make_shared<String>();

std::string Array::toString() const { return fmt::format("array<{}>", this->internal->toString()); }

bool Array::isConvertible(const Type &rhs) const
{
    if (auto arr = dynamic_cast<const Array *>(&rhs)) {
        return this->internal->isConvertible(*arr->internal);
    }
    return false;
}

std::string Object::toString() const
{
    return fmt::format("object<{}>", this->internal->toString());
}

bool Object::isConvertible(const Type &rhs) const
{
    if (auto obj = dynamic_cast<const Object *>(&rhs)) {
        return this->internal->isConvertible(*obj->internal);
    } else if (auto stru = dynamic_cast<const Struct *>(&rhs)) {
        for (const auto &[k, v]: stru->fields) {
            if (!this->internal->isConvertible(*v)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

std::string Struct::toString() const
{
    std::ostringstream ss;
    ss << "struct{ ";
    for (const auto &[k, v]: this->fields) {
        ss << k << ": " << v->toString() << "; ";
    }
    ss << "}";
    return ss.str();
}

bool Struct::isConvertible(const Type &rhs) const
{
    if (auto stru = dynamic_cast<const Struct *>(&rhs)) {
        for (const auto &[k, v]: this->fields) {
            auto it = stru->fields.find(k);
            if (it == stru->fields.end() || !v->isConvertible(*it->second)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

std::string Function::toString() const
{
    std::ostringstream ss;
    ss << "(";
    bool first = true;
    for (const auto &v: this->args) {
        ss << (first ? "" : ", ") << v->toString();
        first = false;
    }
    ss << ") => " << this->ret->toString();
    return ss.str();
}

bool Function::isConvertible(const Type &rhs) const
{
    if (auto func = dynamic_cast<const Function *>(&rhs)) {
        if (this->args.size() != func->args.size()) {
            return false;
        }
        for (int i = 0; i < this->args.size(); ++i) {
            if (!this->args.at(i)->isConvertible(*func->args.at(i))) {
                return false;
            }
        }
        if (!this->ret->isConvertible(*func->ret)) {
            return false;
        }
        return true;
    }
    return false;
}

}  // namespace type
