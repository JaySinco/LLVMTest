#include "udp.h"
#include "ipv4.h"

namespace net
{

std::map<uint16_t, Protocol::Type> Udp::table = {
    {53, kDNS},
};

Udp::Udp(BytesReader& reader)
{
    sport_ = reader.read16u();
    dport_ = reader.read16u();
    tlen_ = reader.read16u();
    crc_ = reader.read16u(false);
}

void Udp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Udp>(reader);
    stack.push(p);

    switch (p->udpType()) {
        case kDNS:
        default:
            Unimplemented::decode(reader, stack);
            break;
    }
}

void Udp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    tlen_ = kFixedBytes + bytes.size();
    auto ipv4 = std::dynamic_pointer_cast<Ipv4>(stack.get(kIPv4));
    crc_ = overallChecksum(ipv4->sip(), ipv4->dip(), bytes.data(), bytes.size());
    BytesBuilder builder;
    encodeHeader(builder);
    builder.prependTo(bytes);
}

Json Udp::toJson() const
{
    Json j;
    j["type"] = type();
    Type udp_type = udpType();
    j["udp-type"] = udp_type == kUnknown ? "unknown" : descType(udp_type);
    j["source-port"] = sport_;
    j["dest-port"] = dport_;
    j["total-size"] = tlen_;
    j["checksum"] = crc_;
    return j;
}

Protocol::Type Udp::type() const { return kUDP; }

Protocol::Type Udp::udpType() const
{
    if (table.count(dport_) != 0) {
        return table.at(dport_);
    }
    if (table.count(sport_) != 0) {
        return table.at(sport_);
    }
    return kUnknown;
}

bool Udp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Udp const&>(resp);
        return sport_ == p.dport_ && dport_ == p.sport_;
    }
    return false;
}

void Udp::encodeHeader(BytesBuilder& builder) const
{
    builder.write16u(sport_);
    builder.write16u(dport_);
    builder.write16u(tlen_);
    builder.write16u(crc_, false);
}

uint16_t Udp::overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const
{
    BytesBuilder builder;
    builder.writeIp4(sip);  // pseudo header
    builder.writeIp4(dip);
    builder.write8u(0);
    builder.write8u(17);
    builder.write16u(tlen_);
    encodeHeader(builder);              // udp header
    builder.writeBytes(payload, size);  // data
    return checksum(builder.data(), builder.size());
}

}  // namespace net