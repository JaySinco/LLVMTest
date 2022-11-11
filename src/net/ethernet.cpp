#include "ethernet.h"
#include "platform.h"
#include "arp.h"
#include "ipv4.h"

namespace net
{

std::map<uint16_t, Protocol::Type> Ethernet::type_dict = {
    {0x0800, Protocol::kIPv4},
    {0x86dd, Protocol::kIPv6},
    {0x0806, Protocol::kARP},
    {0x8035, Protocol::kRARP},
};

void Ethernet::fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack)
{
    auto p = std::make_shared<Ethernet>();
    p->h_ = ntoh(*reinterpret_cast<Header const*>(data));
    stack.push(p);
    data += sizeof(Header);
    size -= sizeof(Header);

    switch (p->ethernetType()) {
        case kIPv4:
            Ipv4::fromBytes(data, size, stack);
            break;
        case kARP:
        case kRARP:
            Arp::fromBytes(data, size, stack);
            break;
        case kIPv6:
        default:
            break;
    }
}

Ethernet::Ethernet(Mac const& smac, Mac const& dmac, Type eth_type)
{
    bool found = false;
    for (auto [k, v]: type_dict) {
        if (v == eth_type) {
            found = true;
            h_.type = k;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error(fmt::format("invalid ethernet type: {}", descType(eth_type)));
    }
    h_.dmac = dmac;
    h_.smac = smac;
}

void Ethernet::toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    auto hd = hton(h_);
    auto it = reinterpret_cast<uint8_t const*>(&hd);
    bytes.insert(bytes.cbegin(), it, it + sizeof(h_));
}

Json Ethernet::toJson() const
{
    Json j;
    j["type"] = descType(type());
    Type eth_type = ethernetType();
    j["ethernet-type"] =
        eth_type == kUnknown ? fmt::format("unknown(0x{:x})", h_.type) : descType(eth_type);
    j["source-mac"] = h_.smac.toStr();
    j["dest-mac"] = h_.dmac.toStr();
    return j;
}

Protocol::Type Ethernet::type() const { return kEthernet; }

bool Ethernet::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Ethernet const&>(resp);
        return p.h_.dmac == Mac::kBroadcast || h_.smac == p.h_.dmac;
    }
    return false;
}

Protocol::Type Ethernet::ethernetType() const
{
    if (type_dict.count(h_.type) != 0) {
        return type_dict.at(h_.type);
    }
    return kUnknown;
}

Ethernet::Header const& Ethernet::getHeader() const { return h_; }

Ethernet::Header Ethernet::ntoh(Header const& h, bool reverse)
{
    Header hd = h;
    ntohx(hd.type, reverse, s);
    return hd;
}

Ethernet::Header Ethernet::hton(Header const& h) { return ntoh(h, true); }

}  // namespace net