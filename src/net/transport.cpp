#include "transport.h"
#include <pcap.h>

namespace net
{

static char errbuf[PCAP_ERRBUF_SIZE];

Transport::Transport(Adaptor const& apt, int to_ms)
{
    if (pcap_init(PCAP_CHAR_ENC_UTF_8, errbuf) != 0) {
        THROW_(fmt::format("failed to init pcap: {}", errbuf));
    }
    pcap_t* p = pcap_create(apt.name.c_str(), errbuf);
    if (p == nullptr) {
        THROW_(fmt::format("failed to create pcap: {}", errbuf));
    }
    pcap_set_snaplen(p, 65535);
    pcap_set_promisc(p, 1);
    pcap_set_timeout(p, to_ms);
    if (pcap_can_set_rfmon(p)) {
        spdlog::info("pcap set radio frequency monitor mode!");
        pcap_set_rfmon(p, 1);
    }
    int ec = pcap_activate(p);
    if (ec > 0) {
        spdlog::warn("activate pcap: {}", pcap_statustostr(ec));
    } else if (ec < 0) {
        std::string msg;
        if (ec == PCAP_ERROR) {
            msg = pcap_geterr(p);
        } else {
            msg = pcap_statustostr(ec);
        }
        THROW_(fmt::format("failed to activate pcap: {}", msg));
    }
    p_ = p;
}

Transport::~Transport()
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    pcap_close(p);
}

void Transport::send(Packet const& pac) const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    if (pcap_sendpacket(p, pac.bytes.data(), pac.bytes.size()) != 0) {
        THROW_(fmt::format("failed to send packet: {}", pcap_geterr(p)));
    }
}

Packet Transport::recv() const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    pcap_pkthdr* info;
    uint8_t const* data;
    int ec = pcap_next_ex(p, &info, &data);
    if (ec == 0) {
        THROW_("packet buffer timeout expired");
    } else if (ec == PCAP_ERROR) {
        THROW_(fmt::format("failed to receive next packet: {}", pcap_geterr(p)));
    } else if (ec != 1) {
        THROW_(fmt::format("failed to receive next packet: {}", ec));
    }
    Packet pac;
    pac.t_ms = info->ts.tv_sec * 1000 + info->ts.tv_usec / 1000;
    pac.bytes.insert(pac.bytes.begin(), data, data + info->len);
    return pac;
}

}  // namespace net