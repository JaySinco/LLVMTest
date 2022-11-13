#pragma once
#include "type.h"

namespace net
{

class Transport
{
public:
    explicit Transport(Adaptor const& apt, int to_ms = 1000);
    ~Transport();

    void send(Packet const& pac) const;
    Packet recv() const;

private:
    void* p_;
};

}  // namespace net