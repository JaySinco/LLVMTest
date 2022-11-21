#include "driver.h"
#include "platform.h"
#include "ethernet.h"
#include "arp.h"
#include <fmt/chrono.h>
#include <pcap.h>

namespace net
{

static char errbuf[PCAP_ERRBUF_SIZE];

static Adaptor const& selectAdaptor(Ip4 hint)
{
    auto& apts = allAdaptors();
    auto it = std::find_if(apts.begin(), apts.end(), [&](Adaptor const& apt) {
        return apt.mask != Ip4::kNull &&
               (hint != Ip4::kNull ? apt.ip.onSameLAN(hint, apt.mask) : apt.gateway != Ip4::kNull);
    });
    if (it == apts.end()) {
        MY_THROW("no adapter on same LAN as {}", hint.toStr());
    }
    ILOG("{} ({})", it->desc, it->ip.toStr());
    return *it;
}

Driver::Driver(Ip4 hint, int to_ms): Driver(selectAdaptor(hint), to_ms) {}

Driver::Driver(Adaptor const& apt, int to_ms): apt_(apt)
{
    if (pcap_init(PCAP_CHAR_ENC_UTF_8, errbuf) != 0) {
        MY_THROW("failed to init pcap: {}", errbuf);
    }
    pcap_t* p = pcap_create(apt.name.c_str(), errbuf);
    if (p == nullptr) {
        MY_THROW("failed to create pcap: {}", errbuf);
    }
    pcap_set_snaplen(p, 65535);
    pcap_set_promisc(p, 1);
    pcap_set_timeout(p, to_ms);
    if (pcap_can_set_rfmon(p)) {
        ILOG("pcap set radio frequency monitor mode");
        pcap_set_rfmon(p, 1);
    }
    int ec = pcap_activate(p);
    if (ec > 0) {
        WLOG("activate pcap: {}", pcap_statustostr(ec));
    } else if (ec < 0) {
        std::string msg;
        if (ec == PCAP_ERROR) {
            msg = pcap_geterr(p);
        } else {
            msg = pcap_statustostr(ec);
        }
        MY_THROW("failed to activate pcap: {}", msg);
    }
    p_ = p;
}

Driver::~Driver()
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    VLOG("close pcap");
    pcap_close(p);
}

void Driver::send(Packet const& pac) const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    if (pcap_sendpacket(p, pac.bytes.data(), pac.bytes.size()) != 0) {
        MY_THROW("failed to send packet: {}", pcap_geterr(p));
    }
}

void Driver::send(ProtocolStack const& stack) const { send(stack.encode()); }

Expected<Packet> Driver::recv() const
{
    auto p = reinterpret_cast<pcap_t*>(p_);
    pcap_pkthdr* info;
    uint8_t const* data;
    int ec = pcap_next_ex(p, &info, &data);
    if (ec == 0) {
        return Error::packetExpired(LOG_FSTR("packet expired"));
    } else if (ec == PCAP_ERROR) {
        return Error::catchAll(LOG_FSTR("{}", pcap_geterr(p)));
    } else if (ec != 1) {
        return Error::catchAll(LOG_FSTR("failed to recv packet: {}", ec));
    }
    Packet pac;
    pac.t_ms = info->ts.tv_sec * 1000 + info->ts.tv_usec / 1000;
    pac.bytes.insert(pac.bytes.begin(), data, data + info->len);
    return pac;
}

Expected<ProtocolStack> Driver::request(ProtocolStack const& req, int to_ms, bool recv_only) const
{
    if (!recv_only) {
        send(req);
    }
    auto start = std::chrono::system_clock::now();
    while (true) {
        if (to_ms > 0) {
            auto now = std::chrono::system_clock::now();
            auto timeout = std::chrono::milliseconds(to_ms);
            auto elasped = now - start;
            if (elasped >= timeout) {
                return Error::timeout(
                    LOG_FSTR("timeout after {}",
                             std::chrono::duration_cast<std::chrono::milliseconds>(elasped)));
            }
        }
        Expected<Packet> pac = recv();
        if (!pac) {
            if (pac.error().packetExpired()) {
                WLOG("skip expired packet");
                continue;
            } else {
                return Error::unexpected(pac.error());
            }
        }
        auto reply = ProtocolStack::decode(*pac);
        // VLOG(reply.toJson().dump(3));
        if (req.correlated(reply)) {
            return reply;
        }
    }
}

Expected<Mac> Driver::getMac(Ip4 ip, bool use_cache, int to_ms) const
{
    static std::map<Ip4, std::pair<Mac, std::chrono::system_clock::time_point>> cached;

    if (ip == apt_.ip) {
        return apt_.mac;
    }

    auto now = std::chrono::system_clock::now();
    if (use_cache) {
        auto it = cached.find(ip);
        if (it != cached.end()) {
            if (now - it->second.second < std::chrono::seconds(30)) {
                WLOG("use cached mac for {}", ip.toStr());
                return it->second.first;
            } else {
                ILOG("cached mac for {} expired, send arp to update", ip.toStr());
            }
        }
    }

    std::atomic<bool> over = false;
    ProtocolStack req = broadcastedARP(apt_.mac, apt_.ip, Mac::kNull, ip);
    Packet pac = req.encode();
    std::thread send_loop([&] {
        MY_TRY;
        while (!over) {
            send(pac);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        MY_CATCH;
    });
    auto thread_guard = utils::scopeExit([&]() {
        over = true;
        send_loop.join();
    });
    Expected<ProtocolStack> reply = request(req, to_ms, true);
    if (!reply) {
        return Error::unexpected(reply.error());
    }
    Mac mac = std::dynamic_pointer_cast<Arp>((*reply).get(Protocol::kARP))->smac();
    cached[ip] = std::make_pair(mac, now);
    return mac;
}

ProtocolStack Driver::broadcastedARP(Mac smac, Ip4 sip, Mac dmac, Ip4 dip, bool reply) const
{
    ProtocolStack stack;
    stack.push(std::make_shared<Ethernet>(apt_.mac, Mac::kBroadcast, Protocol::kARP));
    stack.push(std::make_shared<Arp>(smac, sip, dmac, dip, reply, false));
    return stack;
}

nonstd::unexpected_type<Error> Error::unexpected(Error const& err)
{
    return nonstd::unexpected_type<Error>(err);
}

nonstd::unexpected_type<Error> Error::catchAll(std::string const& msg)
{
    Error err;
    err.flags_ |= kCatchAll;
    err.msg_ = msg;
    return unexpected(err);
}

nonstd::unexpected_type<Error> Error::timeout(std::string const& msg)
{
    Error err;
    err.flags_ |= kTimeout;
    err.msg_ = msg;
    return unexpected(err);
}

nonstd::unexpected_type<Error> Error::packetExpired(std::string const& msg)
{
    Error err;
    err.flags_ |= kPacketExpired;
    err.msg_ = msg;
    return unexpected(err);
}

}  // namespace net