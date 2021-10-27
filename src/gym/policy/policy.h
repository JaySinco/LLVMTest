#pragma once
#include "../prec.h"

class Policy
{
public:
    virtual ~Policy() {}
    virtual torch::Tensor make_action(torch::Tensor observe, bool is_training) = 0;
    virtual void update(torch::Tensor observe, torch::Tensor reward, torch::Tensor done) = 0;
};
