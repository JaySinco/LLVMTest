#pragma once
#include "protocol.h"

namespace net
{

class Ethernet: public Protocol
{
public:
    struct Header
    {
        Mac dmac;       // Destination address
        Mac smac;       // Source address
        uint16_t type;  // Ethernet type
    };

    Ethernet() = default;
    Ethernet(Mac const& smac, Mac const& dmac, Type eth_type);
    ~Ethernet() override = default;

    static void fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack);

    void toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Type ethernetType() const;
    Header const& getHeader() const;

private:
    Header h_{0};
    static std::map<uint16_t, Type> type_dict;
    static Header ntoh(Header const& h, bool reverse = false);
    static Header hton(Header const& h);
};

}  // namespace net
