#include "protocol.h"
#include "ethernet.h"
#include <random>
#include <chrono>

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
    }
    return "";
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

ProtocolStack ProtocolStack::fromPacket(Packet const& pac)
{
    ProtocolStack stack;
    uint8_t const* data = pac.bytes.data();
    size_t size = pac.bytes.size();
    Ethernet::fromBytes(data, size, stack);
    if (size != 0) {
        THROW_(fmt::format("packet bytes not consumed: {}", size));
    }
    return stack;
}

Packet ProtocolStack::toPacket() const
{
    Packet pac;
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        (*it)->toBytes(pac.bytes, *this);
    }
    pac.t_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
    return pac;
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
        THROW_(fmt::format("protocol stack don't have {}", Protocol::descType(type)));
    }
    return std::distance(stack_.begin(), it);
}

void Unimplemented::fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack)
{
    auto p = std::make_shared<Unimplemented>();
    stack.push(p);
    data += size;
    size = 0;
}

void Unimplemented::toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    THROW_("should not be called!");
}

Json Unimplemented::toJson() const
{
    Json j;
    j["type"] = descType(type());
    return j;
}

Protocol::Type Unimplemented::type() const { return kUnknown; };

bool Unimplemented::correlated(Protocol const& resp) const { return false; };

}  // namespace net