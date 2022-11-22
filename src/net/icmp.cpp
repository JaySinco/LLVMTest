#include "icmp.h"

namespace net
{

std::map<std::pair<uint8_t, uint8_t>, std::string> Icmp::table = {
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
    reader.read8u(icmp_type_);
    reader.read8u(code_);
    reader.read16u(crc_, false);

    // Netmask: 17,18
    if (icmp_type_.v == 17 || icmp_type_.v == 18) {
        reader.read16u(id_);
        reader.read16u(sn_);
        reader.readIp4(mask_);
    }
    // Timestamp: 13,14
    else if (icmp_type_.v == 13 || icmp_type_.v == 14) {
        reader.read16u(id_);
        reader.read16u(sn_);
        reader.read32u(init_);
        reader.read32u(recv_);
        reader.read32u(send_);
    }
    // Unreachable: 3
    else if (icmp_type_.v == 3) {
        reader.read32u(unused_);
        eip_ = Ipv4(reader);
        reader.readAll(aft_ip_);
    }
    // Ping: 0,8
    else if (icmp_type_.v == 0 || icmp_type_.v == 8) {
        reader.read16u(id_);
        reader.read16u(sn_);
        reader.readAll(echo_);
    } else {
    }
}

Icmp Icmp::pingAsk(std::string const& echo)
{
    Icmp p;
    p.icmp_type_.v = 8;
    p.code_.v = 0;
    p.crc_.v = 0;
    p.id_.v = rand16u();
    p.sn_.v = rand16u();
    p.echo_.v = echo;
    p.crc_.v = p.overallChecksum();
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
    j["type"] = typeDesc(type());
    JSON_PROP(j, icmp_type_);
    j[icmp_type_t::k]["hint"] = FSTR("desc:{};", icmpDesc());
    JSON_PROP(j, code_);
    JSON_PROP(j, crc_);
    j[crc_t::k]["hint"] = FSTR("checked:{};", overallChecksum());

    // Netmask: 17,18
    if (icmp_type_.v == 17 || icmp_type_.v == 18) {
        JSON_PROP(j, id_);
        JSON_PROP(j, sn_);
        JSON_PROP(j, mask_);
    }
    // Timestamp: 13,14
    else if (icmp_type_.v == 13 || icmp_type_.v == 14) {
        JSON_PROP(j, id_);
        JSON_PROP(j, sn_);
        JSON_PROP(j, init_);
        JSON_PROP(j, recv_);
        JSON_PROP(j, send_);
    }
    // Unreachable: 3
    else if (icmp_type_.v == 3) {
        j["eip"] = eip_.toJson();
    }
    // Ping: 0,8
    else if (icmp_type_.v == 0 || icmp_type_.v == 8) {
        JSON_PROP(j, id_);
        JSON_PROP(j, sn_);
        JSON_PROP(j, echo_);
    } else {
    }
    return j;
}

Protocol::Type Icmp::type() const { return kICMP; }

Protocol::Type Icmp::typeNext() const { return kNull; }

bool Icmp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Icmp const&>(resp);

        // Netmask: 17,18
        if (icmp_type_.v == 17 && p.icmp_type_.v == 18) {
            return id_.v == p.id_.v && sn_.v == p.sn_.v;
        }
        // Timestamp: 13,14
        else if (icmp_type_.v == 13 && p.icmp_type_.v == 14) {
            return id_.v == p.id_.v && sn_.v == p.sn_.v;
        }
        // Unreachable: 3
        else if (icmp_type_.v == 3) {
            return false;
        }
        // Ping: 0,8
        else if (icmp_type_.v == 8 && p.icmp_type_.v == 0) {
            return id_.v == p.id_.v && sn_.v == p.sn_.v;
        } else {
        }
    }
    return false;
}

void Icmp::encodeOverall(BytesBuilder& builder) const
{
    builder.write8u(icmp_type_.v);
    builder.write8u(code_.v);
    builder.write16u(crc_.v, false);

    // Netmask: 17,18
    if (icmp_type_.v == 17 || icmp_type_.v == 18) {
        builder.write16u(id_.v);
        builder.write16u(sn_.v);
        builder.writeIp4(mask_.v);
    }
    // Timestamp: 13,14
    else if (icmp_type_.v == 13 || icmp_type_.v == 14) {
        builder.write16u(id_.v);
        builder.write16u(sn_.v);
        builder.write32u(init_.v);
        builder.write32u(recv_.v);
        builder.write32u(send_.v);
    }
    // Unreachable: 3
    else if (icmp_type_.v == 3) {
        builder.write32u(0);
        eip_.encodeHeader(builder);
        builder.writeBytes(aft_ip_.v);
    }
    // Ping: 0,8
    else if (icmp_type_.v == 0 || icmp_type_.v == 8) {
        builder.write16u(id_.v);
        builder.write16u(sn_.v);
        builder.writeBytes(echo_.v);
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
    auto key = std::make_pair(icmp_type_.v, code_.v);
    return table.count(key) > 0 ? table.at(key) : FSTR("unknown({}, {}})", icmp_type_.v, code_.v);
}

}  // namespace net