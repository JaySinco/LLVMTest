#include "icmp.h"

namespace net
{

std::map<std::pair<uint8_t, uint8_t>, std::string> table = {
    {{0, 0}, "ping reply"},
    {{3, 0}, "network unreachable"},
    {{3, 1}, "host unreachable"},
    {{3, 2}, "protocol unreachable"},
    {{3, 3}, "port unreachable"},
    {{3, 4}, "fragmentation needed but forbid-slice bit set"},
    {{3, 5}, "source routing failed"},
    {{3, 6}, "destination network unknown"},
    {{3, 7}, "destination host unknown"},
    {{3, 8}, "source host isolated (obsolete)"},
    {{3, 9}, "destination network administratively prohibited"},
    {{3, 10}, "destination host administratively prohibited"},
    {{3, 11}, "network unreachable for TOS"},
    {{3, 12}, "host unreachable for TOS"},
    {{3, 13}, "communication administratively prohibited by filtering"},
    {{3, 14}, "host precedence violation"},
    {{3, 15}, "precedence cutoff in effect"},
    {{4, 0}, "source quench"},
    {{5, 0}, "redirect for network"},
    {{5, 1}, "redirect for host"},
    {{5, 2}, "redirect for TOS and network"},
    {{5, 3}, "redirect for TOS and host"},
    {{8, 0}, "ping ask"},
    {{9, 0}, "router notice"},
    {{10, 0}, "router request"},
    {{11, 0}, "ttl equals 0 during transit"},
    {{11, 1}, "ttl equals 0 during reassembly"},
    {{12, 0}, "ip header bad (catch-all error)"},
    {{12, 1}, "required options missing"},
    {{13, 0}, "timestamp ask"},
    {{14, 0}, "timestamp reply"},
    {{15, 0}, "information ask (deprecated)"},
    {{16, 0}, "information reply (deprecated)"},
    {{17, 0}, "netmask ask"},
    {{18, 0}, "netmask reply"},
};

Icmp::Icmp(BytesReader& reader)
{
    type_ = reader.read8u();
    code_ = reader.read8u();
    crc_ = reader.read16u(false);

    // Netmask: 17,18
    if (type_ == 17 || type_ == 18) {
        id_ = reader.read16u();
        sn_ = reader.read16u();
        mask_ = reader.readIp4();
    }
    // Timestamp: 13,14
    else if (type_ == 13 || type_ == 14) {
        id_ = reader.read16u();
        sn_ = reader.read16u();
        init_ = reader.read32u();
        recv_ = reader.read32u();
        send_ = reader.read32u();
    }
    // Unreachable: 3
    else if (type_ == 3) {
        uint32_t unused = reader.read32u();
        eip_ = Ipv4(reader);
        buf_ = reader.readAll();
    }
    // Ping: 0,8
    else if (type_ == 0 || type_ == 8) {
        id_ = reader.read16u();
        sn_ = reader.read16u();
        buf_ = reader.readAll();
    } else {
    }
}

Icmp Icmp::pingAsk(std::string const& echo)
{
    Icmp p;
    p.type_ = 8;
    p.code_ = 0;
    p.crc_ = 0;
    p.id_ = rand16u();
    p.sn_ = rand16u();
    p.buf_ = std::vector<uint8_t>(echo.begin(), echo.end());
    p.crc_ = p.overallChecksum();
    return p;
}

void Icmp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Icmp>(reader);
    stack.push(p);
}

void Icmp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    BytesBuilder builder;
    encodeOverall(builder);
    builder.prependTo(bytes);
}

Json Icmp::toJson() const
{
    Json j;
    j["type"] = descType(type());
    j["desc"] = icmpDesc();
    j["checksum"] = overallChecksum();

    // Netmask: 17,18
    if (type_ == 17 || type_ == 18) {
        j["id"] = id_;
        j["serial-no"] = sn_;
        j["netmask"] = mask_.toStr();
    }
    // Timestamp: 13,14
    else if (type_ == 13 || type_ == 14) {
        j["id"] = id_;
        j["serial-no"] = sn_;
        j["initiate-timestamp"] = init_;
        j["receive-timestamp"] = recv_;
        j["send-timestamp"] = send_;
    }
    // Unreachable: 3
    else if (type_ == 3) {
        j["ipv4"] = eip_.toJson();
    }
    // Ping: 0,8
    else if (type_ == 0 || type_ == 8) {
        j["id"] = id_;
        j["serial-no"] = sn_;
        j["echo"] = std::string(buf_.begin(), buf_.end());
    } else {
    }
    return j;
}

Protocol::Type Icmp::type() const { return kICMP; }

bool Icmp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Icmp const&>(resp);

        // Netmask: 17,18
        if (type_ == 17 && p.type_ == 18) {
            return id_ == p.id_ && sn_ == p.sn_;
        }
        // Timestamp: 13,14
        else if (type_ == 13 && p.type_ == 14) {
            return id_ == p.id_ && sn_ == p.sn_;
        }
        // Unreachable: 3
        else if (type_ == 3) {
            return false;
        }
        // Ping: 0,8
        else if (type_ == 8 && p.type_ == 0) {
            return id_ == p.id_ && sn_ == p.sn_;
        } else {
        }
    }
    return false;
}

void Icmp::encodeOverall(BytesBuilder& builder) const
{
    builder.write8u(type_);
    builder.write8u(code_);
    builder.write16u(crc_, false);

    // Netmask: 17,18
    if (type_ == 17 || type_ == 18) {
        builder.write16u(id_);
        builder.write16u(sn_);
        builder.writeIp4(mask_);
    }
    // Timestamp: 13,14
    else if (type_ == 13 || type_ == 14) {
        builder.write16u(id_);
        builder.write16u(sn_);
        builder.write32u(init_);
        builder.write32u(recv_);
        builder.write32u(send_);
    }
    // Unreachable: 3
    else if (type_ == 3) {
        builder.write32u(0);
        eip_.encodeHeader(builder);
        builder.writeBytes(buf_);
    }
    // Ping: 0,8
    else if (type_ == 0 || type_ == 8) {
        builder.write16u(id_);
        builder.write16u(sn_);
        builder.writeBytes(buf_);
    } else {
    }
}

uint16_t Icmp::overallChecksum() const
{
    BytesBuilder builder;
    encodeOverall(builder);
    return checksum(builder.data(), builder.size());
}

std::string Icmp::icmpDesc() const
{
    auto key = std::make_pair(type_, code_);
    return table.count(key) > 0 ? table.at(key) : fmt::format("unknown({}, {}})", type_, code_);
}

}  // namespace net