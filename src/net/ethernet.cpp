#include "ethernet.h"
#include "platform.h"
#include "arp.h"

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

    Type type = type_dict.at(p->h_.type);
    switch (type) {
        case kIPv4:
        case kIPv6:
        case kARP:
        case kRARP:
            Arp::fromBytes(data, size, stack);
            break;
        default:
            throw std::runtime_error(fmt::format("invalid ethernet type: {}", descType(type)));
    }
}

Ethernet::Ethernet(Mac const& smac, Mac const& dmac, Type type)
{
    bool found = false;
    for (auto it: type_dict) {
        if (it.second == type) {
            found = true;
            h_.type = it.first;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error(fmt::format("invalid ethernet type: {}", descType(type)));
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
    j["ethernet-type"] = descType(type_dict.at(h_.type));
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

Ethernet::Header const& Ethernet::getHeader() const { return h_; }

Ethernet::Header Ethernet::ntoh(Header const& h, bool reverse)
{
    Header hd = h;
    ntohx(hd.type, reverse, s);
    return hd;
}

Ethernet::Header Ethernet::hton(Header const& h) { return ntoh(h, true); }

}  // namespace net