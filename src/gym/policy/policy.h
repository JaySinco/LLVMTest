#pragma once
#include "../prec.h"

class Policy
{
public:
    virtual ~Policy() {}
    virtual torch::Tensor act_on(torch::Tensor observe, bool is_training) = 0;
    virtual void update(torch::Tensor action, double reward) = 0;
};
