#include "platform.h"

int main(int argc, char** argv)
{
    MY_TRY;
    utils::initLogger(argv[0]);
    net::Json j;
    for (auto const& apt: net::allAdaptors()) {
        j.push_back(apt.toJson());
    }
    ILOG(j.dump(3));
    MY_CATCH;
}
