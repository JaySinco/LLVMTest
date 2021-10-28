#pragma once
#include "../prec.h"

class Policy
{
public:
    virtual ~Policy() {}
    virtual torch::Tensor make_action(torch::Tensor observe, bool is_training) = 0;
    virtual void update(torch::Tensor observe, torch::Tensor reward, torch::Tensor alive) = 0;
};

struct TensorDataset: public torch::data::Dataset<TensorDataset>
{
    TensorDataset(torch::Tensor data, torch::Tensor target)
        : data(std::move(data)), target(std::move(target))
    {
    }

    ExampleType get(size_t index) override { return {data[index], target[index]}; }

    c10::optional<size_t> size() const override { return data.size(0); }

    torch::Tensor data;
    torch::Tensor target;
};
