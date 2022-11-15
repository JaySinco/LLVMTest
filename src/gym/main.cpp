#include "params.h"

int main(int argc, char** argv)
{
    MY_TRY;
    ILOG("cuda_available={}", torch::cuda::is_available());

    env::Hopper env;
    policy::pg::PG plc(env, params::pg::hopper());
    plc.train();
    plc.eval(true);
    MY_CATCH;
    return 0;
}
