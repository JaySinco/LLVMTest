#include "policy.h"

void Policy::eval()
{
    env.ui_sync([&]() {
        double reward;
        auto ob = env.get_observe();
        auto action = get_action(ob);
        if (bool done = env.step(action, reward)) {
            env.reset();
        }
    });
}
