#pragma once
#include "mujoco-env.h"

class HopperEnv: public MujocoEnv
{
public:
    HopperEnv(bool show_ui): MujocoEnv((__DIRNAME__ / "hopper.xml").string(), 4, show_ui) {}

    int observe_size() const override { return m->nq - 1 + m->nv; }

    torch::Tensor get_observe() override
    {
        mjtNum *buf = new mjtNum[m->nq - 1 + m->nv];
        std::memcpy(buf, d->qpos + 1, sizeof(mjtNum) * (m->nq - 1));
        std::memcpy(buf + m->nq - 1, d->qvel, sizeof(mjtNum) * m->nv);
        auto ob = torch::from_blob(
            buf, {1, m->nq - 1 + m->nv}, [](void *buf) { delete[](mjtNum *) buf; },
            torch::kFloat64);
        ob = ob.to(torch::kFloat32);
        return ob;
    }

    bool step(torch::Tensor action, double &reward) override
    {
        mjtNum posbefore = d->qpos[0];
        do_step(action);
        mjtNum posafter = d->qpos[0];
        double alive_bonus = 1.0;
        reward = (posafter - posbefore) / dt();
        reward += alive_bonus;
        reward -= 1e-3 * torch::sum(torch::square(action)).item<double>();
        bool done = false;
        return done;
    };
};
