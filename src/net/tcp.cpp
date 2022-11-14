#include "tcp.h"
#include "ipv4.h"
#include <boost/algorithm/string/join.hpp>

namespace net
{

std::map<uint16_t, Protocol::Type> Tcp::table = {
    {53, kDNS},
};

Tcp::Tcp(BytesReader& reader)
{
    sport_ = reader.read16u();
    dport_ = reader.read16u();
    sn_ = reader.read32u();
    an_ = reader.read32u();
    hl_flags_ = reader.read16u();
    wlen_ = reader.read16u();
    crc_ = reader.read16u(false);
    urp_ = reader.read16u();
    opts_ = reader.readBytes(headerSize() - kFixedBytes);
}

void Tcp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Tcp>(reader);
    stack.push(p);

    switch (p->tcpType()) {
        case kDNS:
        default:
            Unimplemented::decode(reader, stack);
            break;
    }
}

void Tcp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    auto ipv4 = std::dynamic_pointer_cast<Ipv4>(stack.get(kIPv4));
    crc_ = overallChecksum(ipv4->sip(), ipv4->dip(), bytes.data(), bytes.size());
    BytesBuilder builder;
    encodeHeader(builder);
    builder.prependTo(bytes);
}

Json Tcp::toJson() const
{
    Json j;
    j["type"] = descType(type());
    Type tcp_type = tcpType();
    j["tcp-type"] = tcp_type == kUnknown ? "unknown" : descType(tcp_type);
    j["source-port"] = sport_;
    j["dest-port"] = dport_;
    j["header-size"] = headerSize();
    j["sequence-no"] = sn_;
    j["acknowledge-no"] = an_;
    std::vector<std::string> flags;
    if (hl_flags_ >> 8 & 0x1) {
        flags.push_back("ns");
    }
    if (hl_flags_ >> 7 & 0x1) {
        flags.push_back("cwr");
    }
    if (hl_flags_ >> 6 & 0x1) {
        flags.push_back("ece");
    }
    if (hl_flags_ >> 5 & 0x1) {
        flags.push_back("urg");
    }
    if (hl_flags_ >> 4 & 0x1) {
        flags.push_back("ack");
    }
    if (hl_flags_ >> 3 & 0x1) {
        flags.push_back("psh");
    }
    if (hl_flags_ >> 2 & 0x1) {
        flags.push_back("rst");
    }
    if (hl_flags_ >> 1 & 0x1) {
        flags.push_back("syn");
    }
    if (hl_flags_ & 0x1) {
        flags.push_back("fin");
    }
    j["flags"] = boost::algorithm::join(flags, ";");
    j["window-size"] = wlen_;
    j["checksum"] = crc_;
    j["urgent-pointer"] = urp_;
    return j;
}

Protocol::Type Tcp::type() const { return kTCP; }

Protocol::Type Tcp::tcpType() const
{
    if (table.count(dport_) != 0) {
        return table.at(dport_);
    }
    if (table.count(sport_) != 0) {
        return table.at(sport_);
    }
    return kUnknown;
}

bool Tcp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Tcp const&>(resp);
        return sport_ == p.dport_ && dport_ == p.sport_;
    }
    return false;
}

uint16_t Tcp::headerSize() const { return 4 * (hl_flags_ >> 12 & 0xf); }

void Tcp::encodeHeader(BytesBuilder& builder) const
{
    builder.write16u(sport_);
    builder.write16u(dport_);
    builder.write32u(sn_);
    builder.write32u(an_);
    builder.write16u(hl_flags_);
    builder.write16u(wlen_);
    builder.write16u(crc_, false);
    builder.write16u(urp_);
    builder.writeBytes(opts_);
}

uint16_t Tcp::overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const
{
    BytesBuilder builder;
    builder.writeIp4(sip);  // pseudo header
    builder.writeIp4(dip);
    builder.write8u(0);
    builder.write8u(6);
    builder.write16u(headerSize() + size);
    encodeHeader(builder);              // udp header
    builder.writeBytes(payload, size);  // data
    return checksum(builder.data(), builder.size());
}

}  // namespace net