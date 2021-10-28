#pragma once
#include "policy.h"

namespace sloppy
{
class Sloppy: public Policy
{
public:
    Sloppy(int64_t act_size): act_size(act_size){};

    torch::Tensor get_action(torch::Tensor observe) override { return torch::zeros({1, act_size}); }

    void update(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
                torch::Tensor alive) override
    {
    }

private:
    int64_t act_size;
};

}  // namespace sloppy
