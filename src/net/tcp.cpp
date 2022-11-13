#include "tcp.h"
#include "ipv4.h"
#include <boost/algorithm/string/join.hpp>

std::map<std::string, std::map<u_short, std::string>> protocol::port_dict = {
    {Protocol_Type_TCP,
     {{22, Protocol_Type_SSH},
      {23, Protocol_Type_TELNET},
      {53, Protocol_Type_DNS},
      {80, Protocol_Type_HTTP},
      {443, Protocol_Type_HTTPS},
      {3389, Protocol_Type_RDP}}}};

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

tcp::tcp(u_char const* const start, u_char const*& end, protocol const* prev)
{
    d = ntoh(*reinterpret_cast<detail const*>(start));
    end = start + 4 * (d.hl_flags >> 12 & 0xf);
    auto& ipdt = dynamic_cast<ipv4 const*>(prev)->get_detail();
    extra.len = ipdt.tlen - 4 * (ipdt.ver_hl & 0xf);
    pseudo_header ph;
    ph.sip = ipdt.sip;
    ph.dip = ipdt.dip;
    ph.type = ipdt.type;
    ph.zero_pad = 0;
    ph.len = htons(extra.len);
    size_t tlen = sizeof(pseudo_header) + extra.len;
    u_char* buf = new u_char[tlen];
    std::memcpy(buf, &ph, sizeof(pseudo_header));
    std::memcpy(buf + sizeof(pseudo_header), start, extra.len);
    extra.crc = calc_checksum(buf, tlen);
    delete[] buf;
}

void tcp::to_bytes(std::vector<u_char>& bytes) const { THROW_("unimplemented method"); }

json tcp::to_json() const
{
    json j;
    j["type"] = type();
    j["tcp-type"] = succ_type();
    j["source-port"] = d.sport;
    j["dest-port"] = d.dport;
    size_t header_size = 4 * (d.hl_flags >> 12 & 0xf);
    j["header-size"] = header_size;
    j["total-size"] = extra.len;
    j["sequence-no"] = d.sn;
    j["acknowledge-no"] = d.an;
    std::vector<std::string> flags;
    if (d.hl_flags >> 8 & 0x1) flags.push_back("ns");
    if (d.hl_flags >> 7 & 0x1) flags.push_back("cwr");
    if (d.hl_flags >> 6 & 0x1) flags.push_back("ece");
    if (d.hl_flags >> 5 & 0x1) flags.push_back("urg");
    if (d.hl_flags >> 4 & 0x1) flags.push_back("ack");
    if (d.hl_flags >> 3 & 0x1) flags.push_back("psh");
    if (d.hl_flags >> 2 & 0x1) flags.push_back("rst");
    if (d.hl_flags >> 1 & 0x1) flags.push_back("syn");
    if (d.hl_flags & 0x1) flags.push_back("fin");
    j["flags"] = boost::algorithm::join(flags, ";");
    j["window-size"] = d.wlen;
    j["checksum"] = extra.crc;
    j["urgent-pointer"] = d.urp;
    return j;
}

std::string tcp::type() const { return Protocol_Type_TCP; }

std::string tcp::succ_type() const
{
    std::string dtype = guess_protocol_by_port(d.dport, Protocol_Type_TCP);
    if (is_specific(dtype)) {
        return dtype;
    }
    return guess_protocol_by_port(d.sport, Protocol_Type_TCP);
}

bool tcp::link_to(protocol const& rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<tcp const&>(rhs);
        return d.sport == p.d.dport && d.dport == p.d.sport;
    }
    return false;
}

tcp::detail const& tcp::get_detail() const { return d; }

tcp::detail tcp::ntoh(detail const& d, bool reverse)
{
    detail dt = d;
    ntohx(dt.sport, !reverse, s);
    ntohx(dt.dport, !reverse, s);
    ntohx(dt.sn, !reverse, l);
    ntohx(dt.an, !reverse, l);
    ntohx(dt.hl_flags, !reverse, s);
    ntohx(dt.wlen, !reverse, s);
    ntohx(dt.urp, !reverse, s);
    return dt;
}

tcp::detail tcp::hton(detail const& d) { return ntoh(d, true); }
