#pragma once
#include "policy.h"

namespace policy
{
class Inaction: public Policy
{
public:
    Inaction(env::Env &env): Policy(env){};

    torch::Tensor get_action(torch::Tensor observe) override
    {
        return torch::zeros({1, env.act_space()});
    }
};

}  // namespace policy
