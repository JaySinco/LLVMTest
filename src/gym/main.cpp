#include "params.h"
#include "utils/args.h"
#include <torch/cuda.h>

int main(int argc, char** argv)
{
    MY_TRY;
    INIT_LOG(argc, argv);
    ILOG("cuda_available={}", torch::cuda::is_available());
    env::Hopper env;
    policy::pg::PG plc(env, params::pg::hopper());
    plc.train();
    plc.eval(true);
    MY_CATCH;
    return 0;
}
