#pragma once
#include "policy.h"

class Inaction: public Policy
{
public:
    Inaction(Env &env): Policy(env){};

    torch::Tensor get_action(torch::Tensor observe) override
    {
        return torch::zeros({1, env.act_space()});
    }
};
