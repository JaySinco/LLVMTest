#include "udp.h"
#include "ipv4.h"
#include "dns.h"

namespace net
{

std::map<uint16_t, Protocol::Type> Udp::table = {
    {22, kSSH},
    {23, kTelnet},
    {53, kDNS},
    {80, kHTTP},
};

Udp::Udp(BytesReader& reader)
{
    reader.read16u(sport_);
    reader.read16u(dport_);
    reader.read16u(tlen_);
    reader.read16u(crc_, false);
}

void Udp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Udp>(reader);
    stack.push(p);

    switch (p->typeNext()) {
        case kDNS:
            Dns::decode(reader, stack);
            break;
        default:
            Unimplemented::decode(reader, stack);
            break;
    }
}

void Udp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    tlen_.v = kFixedBytes + bytes.size();
    auto ipv4 = std::dynamic_pointer_cast<Ipv4>(stack.get(kIPv4));
    crc_.v = overallChecksum(ipv4->sip(), ipv4->dip(), bytes.data(), bytes.size());
    BytesBuilder builder;
    encodeHeader(builder);
    builder.prependTo(bytes);
}

Json Udp::toJson() const
{
    Json j;
    j["type"] = typeDesc(type());
    JSON_PROP(j, sport_);
    JSON_PROP(j, dport_);
    JSON_PROP(j, tlen_);
    JSON_PROP(j, crc_);
    return j;
}

Protocol::Type Udp::type() const { return kUDP; }

Protocol::Type Udp::typeNext() const
{
    if (table.count(dport_.v) != 0) {
        return table.at(dport_.v);
    }
    if (table.count(sport_.v) != 0) {
        return table.at(sport_.v);
    }
    return kUnknown;
}

bool Udp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Udp const&>(resp);
        return sport_.v == p.dport_.v && dport_.v == p.sport_.v;
    }
    return false;
}

void Udp::encodeHeader(BytesBuilder& builder) const
{
    builder.write16u(sport_.v);
    builder.write16u(dport_.v);
    builder.write16u(tlen_.v);
    builder.write16u(crc_.v, false);
}

uint16_t Udp::overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const
{
    BytesBuilder builder;
    builder.writeIp4(sip);  // pseudo header
    builder.writeIp4(dip);
    builder.write8u(0);
    builder.write8u(17);
    builder.write16u(tlen_.v);
    encodeHeader(builder);              // udp header
    builder.writeBytes(payload, size);  // data
    return checksum(builder.data(), builder.size());
}

}  // namespace net