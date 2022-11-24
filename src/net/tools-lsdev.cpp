#include "platform.h"
#include "utils/args.h"

int main(int argc, char** argv)
{
    MY_TRY;
    INIT_LOG(argc, argv);
    net::Json j;
    for (auto const& apt: net::allAdaptors()) {
        j.push_back(apt.toJson());
    }
    ILOG(j.dump(3));
    MY_CATCH;
}
