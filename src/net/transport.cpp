#include "transport.h"
#include "ethernet.h"
#include "arp.h"
#include <pcap.h>

namespace net
{

static char errbuf[PCAP_ERRBUF_SIZE];

Transport::Transport(Adaptor const& apt, int to_ms): apt_(apt)
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

utils::Expected<Packet> Transport::recv() const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    pcap_pkthdr* info;
    uint8_t const* data;
    int ec = pcap_next_ex(p, &info, &data);
    if (ec == 0) {
        return utils::unexpected("packet buffer timeout expired");
    } else if (ec == PCAP_ERROR) {
        return utils::unexpected(pcap_geterr(p));
    } else if (ec != 1) {
        return utils::unexpected(fmt::format("failed to receive next packet: {}", ec));
    }
    Packet pac;
    pac.t_ms = info->ts.tv_sec * 1000 + info->ts.tv_usec / 1000;
    pac.bytes.insert(pac.bytes.begin(), data, data + info->len);
    return pac;
}

ProtocolStack Transport::request(ProtocolStack const& req, int to_ms, bool skip_send)
{
    if (!skip_send) {
        send(req.encode());
    }
    auto start = std::chrono::system_clock::now();
    while (true) {
        if (to_ms > 0) {
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            if (ms.count() >= to_ms) {
                THROW_(fmt::format("request timeout after {}ms", to_ms));
            }
        }
        utils::Expected<Packet> pac = recv();
        if (!pac) {
            spdlog::warn(pac.error().what());
            continue;
        }
        auto reply = ProtocolStack::decode(*pac);
        if (req.correlated(reply)) {
            return reply;
        }
    }
}

Mac Transport::getMacFromIp4(Ip4 const& ip, bool use_cache, int to_ms)
{
    static std::map<Ip4, std::pair<Mac, std::chrono::system_clock::time_point>> cached;
    auto now = std::chrono::system_clock::now();
    if (use_cache) {
        auto it = cached.find(ip);
        if (it != cached.end()) {
            if (now - it->second.second < std::chrono::seconds(30)) {
                spdlog::debug("use cached mac for {}", ip.toStr());
                return it->second.first;
            } else {
                spdlog::debug("cached mac for {} expired, send arp to update", ip.toStr());
            }
        }
    }

    std::atomic<bool> over = false;
    ProtocolStack req = stackARP(apt_.mac, apt_.ip, Mac::kNull, ip);
    Packet pac = req.encode();
    std::thread send_loop([&] {
        TRY_;
        while (!over) {
            send(pac);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        CATCH_;
    });
    ProtocolStack reply = request(req, to_ms, true);
    Mac mac = std::dynamic_pointer_cast<Arp>(reply.get(Protocol::kARP))->smac();
    cached[ip] = std::make_pair(mac, now);
    over = true;
    send_loop.join();
    return mac;
}

ProtocolStack Transport::stackARP(Mac const& smac, Ip4 const& sip, Mac const& dmac, Ip4 const& dip,
                                  bool reply, bool reverse)
{
    ProtocolStack stack;
    stack.push(std::make_shared<Ethernet>(smac, Mac::kBroadcast,
                                          reverse ? Protocol::kRARP : Protocol::kARP));
    stack.push(std::make_shared<Arp>(smac, sip, dmac, dip, reply, reverse));
    return stack;
}

}  // namespace net