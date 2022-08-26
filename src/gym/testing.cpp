#include "params.h"

int main(int argc, char** argv)
{
    TRY_;
    env::Hopper env;
    policy::pg::PG plc(env, params::pg::hopper());
    plc.train();
    plc.eval(true);
    CATCH_;
    return 0;
}
