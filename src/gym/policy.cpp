#include "policy.h"

namespace policy
{
void Policy::eval(bool keep_going)
{
    env.reset(true);
    env.uiSync([&]() {
        double reward;
        auto ob = env.getObserve();
        auto action = getAction(ob);
        bool done = env.step(action, reward);
        if (!keep_going && done) {
            env.reset();
        }
    });
}

}  // namespace policy
