#pragma once
#include "../env/env.h"

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
    Policy(Env &env): env(env){};
    virtual ~Policy() {}
    virtual void train() = 0;
    virtual void eval();
    virtual torch::Tensor get_action(torch::Tensor observe) = 0;

protected:
    Env &env;
};
