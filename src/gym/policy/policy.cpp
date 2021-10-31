#include "policy.h"

namespace policy
{
void Policy::eval(bool keep_going)
{
    env.reset(true);
    env.ui_sync([&]() {
        double reward;
        auto ob = env.get_observe();
        auto action = get_action(ob);
        bool done = env.step(action, reward);
        if (!keep_going && done) {
            env.reset();
        }
    });
}

}  // namespace policy
