#pragma once
#include "policy.h"

namespace policy
{
class Inaction: public Policy
{
public:
    explicit Inaction(env::Env& env): Policy(env) {}

    torch::Tensor getAction(torch::Tensor observe) override
    {
        return torch::zeros({1, env.actSpace()});
    }
};

}  // namespace policy
