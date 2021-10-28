#pragma once
#include "policy.h"

namespace sloppy
{
class Sloppy: public Policy
{
public:
    Sloppy(int64_t act_size): act_size(act_size){};

    torch::Tensor make_action(torch::Tensor observe, bool is_training) override
    {
        return torch::zeros({1, act_size});
    }

    void update(torch::Tensor observe, torch::Tensor reward, torch::Tensor alive) override {}

private:
    int64_t act_size;
};

}  // namespace sloppy
