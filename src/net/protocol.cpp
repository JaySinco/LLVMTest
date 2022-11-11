#include "protocol.h"
#include "ethernet.h"
#include <random>

namespace net
{

std::string Protocol::descType(Type type)
{
    switch (type) {
        case kUnknown:
            return "unknown";
        case kEthernet:
            return "ethernet";
        case kIPv4:
            return "ipv4";
        case kIPv6:
            return "ipv6";
        case kARP:
            return "arp";
        case kRARP:
            return "rarp";
        case kICMP:
            return "icmp";
        case kTCP:
            return "tcp";
        case kUDP:
            return "udp";
        case kDNS:
            return "dns";
        case kHTTP:
            return "http";
        case kHTTPS:
            return "https";
        case kSSH:
            return "ssh";
        case kTelnet:
            return "telnet";
        case kRDP:
            return "rdp";
    }
    return "unimplemented";
}

uint16_t Protocol::checksum(void const* data, size_t size)
{
    uint32_t sum = 0;
    auto buf = static_cast<uint16_t const*>(data);
    while (size > 1) {
        sum += *buf++;
        size -= 2;
    }
    if (size > 0) {
        uint16_t left = 0;
        std::memcpy(&left, buf, 1);
        sum += left;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return (static_cast<uint16_t>(sum) ^ 0xffff);
}

uint16_t Protocol::rand16u()
{
    static std::random_device rd;
    static std::default_random_engine engine(rd());
    static std::uniform_int_distribution<uint16_t> dist;
    return dist(engine);
}

ProtocolStack ProtocolStack::fromBytes(uint8_t const* data, size_t size)
{
    ProtocolStack stack;
    Ethernet::fromBytes(data, size, stack);
    return stack;
}

std::vector<uint8_t> ProtocolStack::toBytes() const
{
    std::vector<uint8_t> data;
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        (*it)->toBytes(data, *this);
    }
    return data;
}

Json ProtocolStack::toJson() const
{
    Json j;
    for (auto& p: stack_) {
        j.push_back(p->toJson());
    }
    return j;
}

bool ProtocolStack::correlated(ProtocolStack const& resp) const
{
    if (size() != resp.size()) {
        return false;
    }
    for (int i = 0; i < size(); ++i) {
        if (!get(i)->correlated(*resp.get(i))) {
            return false;
        }
    }
    return true;
}

ProtocolPtr ProtocolStack::get(Protocol::Type type) const { return get(getIdx(type)); }

size_t ProtocolStack::getIdx(Protocol::Type type) const
{
    auto it = std::find_if(stack_.begin(), stack_.end(),
                           [&](ProtocolPtr p) { return p->type() == type; });
    if (it == stack_.end()) {
        throw std::runtime_error(
            fmt::format("protocol stack don't have {}", Protocol::descType(type)));
    }
    return std::distance(stack_.begin(), it);
}

}  // namespace net