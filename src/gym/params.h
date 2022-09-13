#pragma once
#include "hopper.h"
#include "inaction.h"
#include "pg.h"

namespace params
{
namespace pg
{
inline policy::pg::HyperParams hopper()
{
    policy::pg::HyperParams hp;
    hp.max_iters = 15000;
    hp.sampling_steps = 10240;
    hp.minibatch_size = 512;
    hp.epochs = 1;
    hp.hidden = 64;
    hp.log_std = 0;
    hp.gamma = 0.99;
    hp.lr = 3e-4;
    return hp;
}

}  // namespace pg
}  // namespace params
