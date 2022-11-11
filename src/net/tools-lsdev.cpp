#include "utils/base.h"
#include "platform.h"

int main(int argc, char** argv)
{
    TRY_;
    net::Json j;
    for (auto const& apt: net::allAdaptors()) {
        j.push_back(apt.toJson());
    }
    spdlog::info(j.dump(3));
    CATCH_;
}
