#include "platform.h"
#include <pcap.h>

namespace net
{

static char errbuf[PCAP_ERRBUF_SIZE];

Driver::Driver(Adaptor const& apt, int to_ms)
{
    if (pcap_init(PCAP_CHAR_ENC_UTF_8, errbuf) != 0) {
        throw std::runtime_error(fmt::format("failed to init pcap: {}", errbuf));
    }
    pcap_t* p = pcap_create(apt.name.c_str(), errbuf);
    if (p == nullptr) {
        throw std::runtime_error(fmt::format("failed to create pcap: {}", errbuf));
    }
    pcap_set_snaplen(p, 65535);
    pcap_set_promisc(p, 1);
    pcap_set_timeout(p, to_ms);
    if (pcap_can_set_rfmon(p)) {
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
        throw std::runtime_error(fmt::format("failed to activate pcap: {}", msg));
    }
    p_ = p;
}

Driver::~Driver()
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    pcap_close(p);
}

void Driver::send(Packet const& pac) const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    if (pcap_sendpacket(p, pac.bytes.data(), pac.bytes.size()) != 0) {
        throw std::runtime_error(fmt::format("failed to send packet: {}", pcap_geterr(p)));
    }
}

Packet Driver::recv() const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    pcap_pkthdr* info;
    uint8_t const* data;
    int ec = pcap_next_ex(p, &info, &data);
    if (ec == 0) {
        throw std::runtime_error("packet buffer timeout expired");
    } else if (ec == PCAP_ERROR) {
        throw std::runtime_error(fmt::format("failed to receive next packet: {}", pcap_geterr(p)));
    } else if (ec != 1) {
        throw std::runtime_error(fmt::format("failed to receive next packet: {}", ec));
    }
}

}  // namespace net