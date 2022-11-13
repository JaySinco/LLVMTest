#include "icmp.h"

namespace net
{

std::map<std::pair<uint8_t, uint8_t>, std::string> type_dict = {
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

icmp::icmp(u_char const* const start, u_char const*& end, protocol const* prev)
{
    d = ntoh(*reinterpret_cast<detail const*>(start));
    auto ip = dynamic_cast<ipv4 const*>(prev);
    extra.raw = std::string(start + sizeof(detail), start + ip->payload_size());
    if (icmp_type() == "error") {
        u_char const* pend = end;
        extra.eip = ipv4(start + sizeof(detail), pend);
        std::memcpy(&extra.buf, pend, 8);
    }
}

icmp::icmp(std::string const& ping_echo)
{
    d.type = 8;
    d.code = 0;
    d.u.s.id = rand_ushort();
    d.u.s.sn = rand_ushort();
    extra.raw = ping_echo;
}

void icmp::to_bytes(std::vector<u_char>& bytes) const
{
    auto dt = hton(d);
    size_t tlen = sizeof(detail) + extra.raw.size();
    u_char* buf = new u_char[tlen];
    std::memcpy(buf, &dt, sizeof(detail));
    std::memcpy(buf + sizeof(detail), extra.raw.data(), extra.raw.size());
    dt.crc = calc_checksum(buf, tlen);
    const_cast<icmp&>(*this).d.crc = dt.crc;
    std::memcpy(buf, &dt, sizeof(detail));
    bytes.insert(bytes.cbegin(), buf, buf + tlen);
    delete[] buf;
}

json icmp::to_json() const
{
    json j;
    j["type"] = type();
    std::string tp = icmp_type();
    j["icmp-type"] = tp;
    if (type_dict.count(d.type) > 0) {
        auto& code_dict = type_dict.at(d.type).second;
        if (code_dict.count(d.code) > 0) {
            j["desc"] = code_dict.at(d.code);
        }
    }
    j["id"] = d.u.s.id;
    j["serial-no"] = d.u.s.sn;
    size_t tlen = sizeof(detail) + extra.raw.size();
    u_char* buf = new u_char[tlen];
    auto dt = hton(d);
    std::memcpy(buf, &dt, sizeof(detail));
    std::memcpy(buf + sizeof(detail), extra.raw.data(), extra.raw.size());
    j["checksum"] = calc_checksum(buf, tlen);
    delete[] buf;

    if (tp == "ping-reply" || tp == "ping-ask") {
        j["echo"] = extra.raw;
    }
    if (tp == "error") {
        json ep;
        ep["ipv4"] = extra.eip.to_json();
        auto error_type = extra.eip.succ_type();
        if (error_type == Protocol_Type_TCP || error_type == Protocol_Type_UDP) {
            ep["source-port"] = ntohs(*reinterpret_cast<u_short const*>(&extra.buf[0]));
            ep["dest-port"] =
                ntohs(*reinterpret_cast<u_short const*>(&extra.buf[0] + sizeof(u_short)));
        }
        j["error-header"] = ep;
    }
    return j;
}

std::string icmp::type() const { return Protocol_Type_ICMP; }

std::string icmp::succ_type() const { return Protocol_Type_Void; }

bool icmp::link_to(protocol const& rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<icmp const&>(rhs);
        return d.u.s.id == p.d.u.s.id && d.u.s.sn == p.d.u.s.sn;
    }
    return false;
}

icmp::detail const& icmp::get_detail() const { return d; }

icmp::extra_detail const& icmp::get_extra() const { return extra; }

std::string icmp::icmp_type() const
{
    return type_dict.count(d.type) > 0 ? type_dict.at(d.type).first : Protocol_Type_Unknow(d.type);
}

icmp::detail icmp::ntoh(detail const& d, bool reverse)
{
    detail dt = d;
    ntohx(dt.u.s.id, !reverse, s);
    ntohx(dt.u.s.sn, !reverse, s);
    return dt;
}

icmp::detail icmp::hton(detail const& d) { return ntoh(d, true); }

}  // namespace net