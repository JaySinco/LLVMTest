#include "arp.h"
#include "platform.h"

namespace net
{

void Arp::fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack)
{
    if (size < sizeof(Header)) {
        THROW_("inadequate bytes for arp header");
    }
    auto p = std::make_shared<Arp>();
    p->h_ = ntoh(*reinterpret_cast<Header const*>(data));
    stack.push(p);
    data += sizeof(Header);
    size -= sizeof(Header);
}

Arp::Arp(Mac const& smac, Ip4 const& sip, Mac const& dmac, Ip4 const& dip, bool reply, bool reverse)
{
    h_.hw_type = 1;
    h_.prot_type = 0x0800;
    h_.hw_len = 6;
    h_.prot_len = 4;
    h_.op = reverse ? (reply ? 4 : 3) : (reply ? 2 : 1);
    h_.smac = smac;
    h_.sip = sip;
    h_.dmac = dmac;
    h_.dip = dip;
}

void Arp::toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    auto hd = hton(h_);
    auto it = reinterpret_cast<uint8_t const*>(&hd);
    bytes.insert(bytes.cbegin(), it, it + sizeof(h_));
}

Json Arp::toJson() const
{
    Json j;
    j["type"] = descType(type());
    j["hardware-type"] = h_.hw_type;
    j["protocol-type"] = h_.prot_type;
    j["hardware-addr-len"] = h_.hw_len;
    j["protocol-addr-len"] = h_.prot_len;
    j["operate"] = (h_.op == 1 || h_.op == 3)   ? "request"
                   : (h_.op == 2 || h_.op == 4) ? "reply"
                                                : fmt::format("invalid(0x{:x})", h_.op);
    j["source-mac"] = h_.smac.toStr();
    j["source-ip"] = h_.sip.toStr();
    j["dest-mac"] = h_.dmac.toStr();
    j["dest-ip"] = h_.dip.toStr();
    return j;
}

Protocol::Type Arp::type() const
{
    return (h_.op == 1 || h_.op == 2) ? kARP : (h_.op == 3 || h_.op == 4) ? kRARP : kUnknown;
}

bool Arp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Arp const&>(resp);
        return (h_.op == 1 || h_.op == 3) && (p.h_.op == 2 || p.h_.op == 4) && (h_.dip == p.h_.sip);
    }
    return false;
}

Arp::Header const& Arp::getHeader() const { return h_; }

Arp::Header Arp::ntoh(Header const& h, bool reverse)
{
    Header hd = h;
    ntohx(hd.hw_type, reverse, s);
    ntohx(hd.prot_type, reverse, s);
    ntohx(hd.op, reverse, s);
    return hd;
}

Arp::Header Arp::hton(Header const& h) { return ntoh(h, true); }

}  // namespace net