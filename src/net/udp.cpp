#include "udp.h"
#include "ipv4.h"

std::map<std::string, std::map<u_short, std::string>> protocol::port_dict = {
    {Protocol_Type_UDP,
     {{22, Protocol_Type_SSH},
      {23, Protocol_Type_TELNET},
      {53, Protocol_Type_DNS},
      {80, Protocol_Type_HTTP}}},
};

std::string protocol::guess_protocol_by_port(u_short port, std::string const& type)
{
    if (port_dict.count(type) > 0) {
        auto& type_dict = port_dict.at(type);
        if (type_dict.count(port) > 0) {
            return type_dict.at(port);
        }
    }
    return Protocol_Type_Unknow(-1);
}

udp::udp(u_char const* const start, u_char const*& end, protocol const* prev)
{
    d = ntoh(*reinterpret_cast<detail const*>(start));
    end = start + sizeof(detail);
    auto& ipdt = dynamic_cast<ipv4 const*>(prev)->get_detail();
    pseudo_header ph;
    ph.sip = ipdt.sip;
    ph.dip = ipdt.dip;
    ph.type = ipdt.type;
    ph.zero_pad = 0;
    ph.len = htons(d.len);
    size_t tlen = sizeof(pseudo_header) + d.len;
    u_char* buf = new u_char[tlen];
    std::memcpy(buf, &ph, sizeof(pseudo_header));
    std::memcpy(buf + sizeof(pseudo_header), start, d.len);
    extra.crc = calc_checksum(buf, tlen);
    delete[] buf;
}

void udp::to_bytes(std::vector<u_char>& bytes) const
{
    throw std::runtime_error("unimplemented method");
}

json udp::to_json() const
{
    json j;
    j["type"] = type();
    j["udp-type"] = succ_type();
    j["source-port"] = d.sport;
    j["dest-port"] = d.dport;
    j["total-size"] = d.len;
    j["checksum"] = extra.crc;
    return j;
}

std::string udp::type() const { return Protocol_Type_UDP; }

std::string udp::succ_type() const
{
    std::string dtype = guess_protocol_by_port(d.dport, Protocol_Type_UDP);
    if (is_specific(dtype)) {
        return dtype;
    }
    return guess_protocol_by_port(d.sport, Protocol_Type_UDP);
}

bool udp::link_to(protocol const& rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<udp const&>(rhs);
        return d.sport == p.d.dport && d.dport == p.d.sport;
    }
    return false;
}

udp::detail const& udp::get_detail() const { return d; }

udp::extra_detail const& udp::get_extra() const { return extra; }

udp::detail udp::ntoh(detail const& d, bool reverse)
{
    detail dt = d;
    ntohx(dt.sport, !reverse, s);
    ntohx(dt.dport, !reverse, s);
    ntohx(dt.len, !reverse, s);
    return dt;
}

udp::detail udp::hton(detail const& d) { return ntoh(d, true); }
