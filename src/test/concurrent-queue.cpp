#include "utils/logging.h"
#include <moodycamel/concurrentqueue.h>

int main(int argc, char** argv)
{
    utils::initLogger(argv[0]);
    moodycamel::ConcurrentQueue<int> q;
    int dequeued[100] = {0};
    std::thread threads[20];

    for (int i = 0; i != 10; ++i) {
        threads[i] = std::thread(
            [&](int i) {
                for (int j = 0; j != 10; ++j) {
                    q.enqueue(i * 10 + j);
                }
            },
            i);
    }

    for (int i = 10; i != 20; ++i) {
        threads[i] = std::thread([&]() {
            int item;
            for (int j = 0; j != 20; ++j) {
                if (q.try_dequeue(item)) {
                    ++dequeued[item];
                    ILOG("dequeue {}", item);
                }
            }
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    int item;
    while (q.try_dequeue(item)) {
        ++dequeued[item];
        ILOG("dequeue_ {}", item);
    }

    for (int i: dequeued) {
        assert(i == 1);
    }
}
