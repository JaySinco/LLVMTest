#pragma once
#include "mujoco-env.h"
#include <numeric>

class HopperEnv: public MujocoEnv
{
public:
    HopperEnv(bool show_ui): MujocoEnv((__DIRNAME__ / "../xml/hopper.xml").string(), 4, show_ui) {}

    int observe_dim() const override { return m->nq - 1 + m->nv; }

    std::vector<double> get_observe() override
    {
        std::vector<double> observe(m->nq - 1 + m->nv);
        std::memcpy(observe.data(), d->qpos + 1, sizeof(mjtNum) * (m->nq - 1));
        std::memcpy(observe.data() + m->nq - 1, d->qvel, sizeof(mjtNum) * m->nv);
        return observe;
    }

    bool step(const std::vector<double> &action, double &reward) override
    {
        mjtNum posbefore = d->qpos[0];
        do_step(action);
        mjtNum posafter = d->qpos[0];
        double alive_bonus = 1.0;
        reward = (posafter - posbefore) / dt();
        reward += alive_bonus;
        reward -= 1e-3 * std::accumulate(action.begin(), action.end(), 0,
                                         [](double acc, double v) { return acc + std::pow(v, 2); });
        bool done = false;
        return done;
    };
};
