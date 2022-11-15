#pragma once
#include "protocol.h"

namespace net
{

class Ethernet: public Protocol
{
public:
    Ethernet() = default;
    Ethernet(Mac smac, Mac dmac, Type eth_type);
    explicit Ethernet(BytesReader& reader);
    ~Ethernet() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Type ethernetType() const;

private:
    Mac dmac_;       // Destination address
    Mac smac_;       // Source address
    uint16_t type_;  // Ethernet type

    static std::map<uint16_t, Type> table;
};

}  // namespace net
