#include "ethernet.h"
#include "platform.h"
#include "arp.h"
#include "ipv4.h"

namespace net
{

std::map<uint16_t, Protocol::Type> Ethernet::table = {
    {0x0800, Protocol::kIPv4},
    {0x86dd, Protocol::kIPv6},
    {0x0806, Protocol::kARP},
    {0x8035, Protocol::kRARP},
};

Ethernet::Ethernet(BytesReader& reader)
{
    dmac_ = reader.readMac();
    smac_ = reader.readMac();
    type_ = reader.read16u();
}

void Ethernet::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Ethernet>(reader);
    stack.push(p);

    switch (p->ethernetType()) {
        case kIPv4:
            Ipv4::decode(reader, stack);
            break;
        case kARP:
        case kRARP:
            Arp::decode(reader, stack);
            break;
        case kIPv6:
        default:
            Unimplemented::decode(reader, stack);
            break;
    }
}

Ethernet::Ethernet(Mac const& smac, Mac const& dmac, Type eth_type)
{
    bool found = false;
    for (auto [k, v]: table) {
        if (v == eth_type) {
            found = true;
            type_ = k;
            break;
        }
    }
    if (!found) {
        THROW_(fmt::format("invalid ethernet type: {}", descType(eth_type)));
    }
    dmac_ = dmac;
    smac_ = smac;
}

void Ethernet::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    BytesBuilder builder;
    builder.writeMac(dmac_);
    builder.writeMac(smac_);
    builder.write16u(type_);
    builder.prependTo(bytes);
}

Json Ethernet::toJson() const
{
    Json j;
    j["type"] = descType(type());
    Type eth_type = ethernetType();
    j["ethernet-type"] =
        eth_type == kUnknown ? fmt::format("unknown(0x{:x})", type_) : descType(eth_type);
    j["source-mac"] = smac_.toStr();
    j["dest-mac"] = dmac_.toStr();
    return j;
}

Protocol::Type Ethernet::type() const { return kEthernet; }

bool Ethernet::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Ethernet const&>(resp);
        return p.dmac_ == Mac::kBroadcast || smac_ == p.dmac_;
    }
    return false;
}

Protocol::Type Ethernet::ethernetType() const
{
    if (table.count(type_) != 0) {
        return table.at(type_);
    }
    return kUnknown;
}

}  // namespace net