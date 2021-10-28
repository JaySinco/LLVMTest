#pragma once
#include "../prec.h"

class Policy
{
public:
    virtual ~Policy() {}
    virtual torch::Tensor get_action(torch::Tensor observe) = 0;
    virtual void update(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
                        torch::Tensor alive) = 0;
};

struct TensorDataset: public torch::data::Dataset<TensorDataset>
{
    TensorDataset(torch::Tensor data, torch::Tensor target): data(data), target(target) {}

    ExampleType get(size_t index) override { return {data[index], target[index]}; }

    c10::optional<size_t> size() const override { return data.size(0); }

    torch::Tensor data;
    torch::Tensor target;
};
