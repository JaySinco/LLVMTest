#include "utils/logging.h"
#include <bshoshany/BS_thread_pool.hpp>

int main(int argc, char** argv)
{
    utils::initLogger(argv[0]);
    BS::thread_pool pool;
    std::future<int> my_future = pool.submit([] { return 42; });
    ILOG(my_future.get());
}
