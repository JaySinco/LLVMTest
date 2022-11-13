#pragma once
#include "type.h"

namespace net
{
class Driver
{
public:
    explicit Driver(Adaptor const& apt, int to_ms = 1000);
    ~Driver();

    void send(Packet const& pac) const;
    Packet recv() const;

private:
    void* p_;
};

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

Ip4 ip4FromDottedDec(std::string const& s);
Ip4 ip4FromDomain(std::string const& s);

std::vector<Adaptor> const& allAdaptors();

}  // namespace net