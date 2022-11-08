#pragma once
#include "env.h"

namespace policy
{
struct TensorDataset: public torch::data::Dataset<TensorDataset>
{
    TensorDataset(torch::Tensor data, torch::Tensor target): data(data), target(target) {}

    ExampleType get(size_t index) override { return {data[index], target[index]}; }

    c10::optional<size_t> size() const override { return data.size(0); }

    torch::Tensor data;
    torch::Tensor target;
};

class Policy
{
public:
    explicit Policy(env::Env& env): env(env) {}
    virtual ~Policy() = default;
    virtual void train() {}
    virtual void eval(bool keep_going = false);
    virtual torch::Tensor get_action(torch::Tensor observe) = 0;

protected:
    env::Env& env;
};

}  // namespace policy
