#include "params.h"

int main(int argc, char** argv)
{
    TRY_;
    spdlog::info("cuda_available={}", torch::cuda::is_available());

    env::Hopper env;
    policy::pg::PG plc(env, params::pg::hopper());
    plc.train();
    plc.eval(true);
    CATCH_;
    return 0;
}
