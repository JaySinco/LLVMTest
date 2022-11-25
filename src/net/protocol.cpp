#include "protocol.h"
#include "ethernet.h"
#include <random>
#include <chrono>
#include <fmt/chrono.h>

namespace net
{

std::string Protocol::typeDesc(Type type)
{
    switch (type) {
        case kNull:
            MY_THROW("<null> should not be described");
        case kUnknown:
            MY_THROW("<unknown> should be described");
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
    MY_THROW("should not reach here");
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

ProtocolStack ProtocolStack::decode(Packet const& pac)
{
    ProtocolStack stack;
    BytesReader reader(pac.bytes);
    Ethernet::decode(reader, stack);
    if (!reader.empty()) {
        MY_THROW("{} bytes not consumed, current stack: {}", reader.size(), stack.toJson().dump(3));
    }
    stack.t_ms_ = pac.t_ms;
    return stack;
}

Packet ProtocolStack::encode() const
{
    Packet pac;
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        (*it)->encode(pac.bytes, *this);
    }
    pac.t_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
    return pac;
}

Json ProtocolStack::toJson() const
{
    Json j;
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        j.push_back((*it)->toJson());
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

size_t ProtocolStack::getIdx(Protocol::Type type) const
{
    auto it = std::find_if(stack_.begin(), stack_.end(),
                           [&](ProtocolPtr p) { return p->type() == type; });
    if (it == stack_.end()) {
        MY_THROW("protocol stack don't have {}", Protocol::typeDesc(type));
    }
    return std::distance(stack_.begin(), it);
}

std::chrono::system_clock::time_point ProtocolStack::getTime() const
{
    return std::chrono::system_clock::time_point(std::chrono::milliseconds(t_ms_));
}

std::string ProtocolStack::getTimeStr() const
{
    auto tm = getTime();
    return FSTR("{:%H:%M:%S}.{:03d}", tm, t_ms_ % 1000);
}

Protocol::Type ProtocolStack::innerMost() const
{
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        auto next = (*it)->typeNext();
        if (next != Protocol::kUnknown && next != Protocol::kNull) {
            return next;
        }
        auto type = (*it)->type();
        if (type != Protocol::kUnknown) {
            return type;
        }
    }
    return Protocol::kUnknown;
}

bool ProtocolStack::has(Protocol::Type type) const
{
    return std::find_if(stack_.begin(), stack_.end(),
                        [&](ProtocolPtr p) { return p->type() == type; }) != stack_.end();
}

void Unimplemented::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Unimplemented>();
    reader.readAll(p->buf_);
    stack.push(p);
}

void Unimplemented::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    MY_THROW("should not be called");
}

Json Unimplemented::toJson() const
{
    Json j;
    j["type"] = "unimplemented";
    JSON_PROP(j, buf_);
    return j;
}

Protocol::Type Unimplemented::type() const { return kUnknown; };

Protocol::Type Unimplemented::typeNext() const { return kNull; };

bool Unimplemented::correlated(Protocol const& resp) const { return false; };

}  // namespace net