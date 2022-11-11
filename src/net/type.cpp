#include "type.h"
#include "platform.h"
#include <sstream>

namespace net
{

const Mac Mac::kNull = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
const Mac Mac::kBroadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool Mac::operator==(Mac const& rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4 && b5 == rhs.b5 &&
           b6 == rhs.b6;
}

bool Mac::operator!=(Mac const& rhs) const { return !(*this == rhs); }

std::string Mac::toStr() const
{
    auto c = reinterpret_cast<uint8_t const*>(this);
    std::ostringstream ss;
    ss << fmt::format("{:02X}", c[0]);
    for (int i = 1; i < 6; ++i) {
        ss << fmt::format("-{:02X}", c[i]);
    }
    return ss.str();
}

const Ip4 Ip4::kNull = {0x0, 0x0, 0x0, 0x0};
const Ip4 Ip4::kBroadcast = {0xff, 0xff, 0xff, 0xff};

Ip4::operator uint32_t() const
{
    auto i = reinterpret_cast<uint32_t const*>(this);
    return ntohl(*i);
}

bool Ip4::operator==(Ip4 const& rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4;
}

bool Ip4::operator!=(Ip4 const& rhs) const { return !(*this == rhs); }

uint32_t Ip4::operator&(Ip4 const& rhs) const
{
    return static_cast<uint32_t>(*this) & static_cast<uint32_t>(rhs);
}

bool Ip4::onSameLAN(Ip4 const& rhs, Ip4 const& mask) const
{
    return (*this & mask) == (rhs & mask);
}

bool Ip4::isSelf() const
{
    auto apts = allAdaptors();
    return std::find_if(apts.begin(), apts.end(),
                        [&](Adaptor const& apt) { return (*this) == apt.ip; }) != apts.end();
}

std::string Ip4::toStr() const
{
    auto c = reinterpret_cast<uint8_t const*>(this);
    std::ostringstream ss;
    ss << static_cast<int>(c[0]);
    for (int i = 1; i < 4; ++i) {
        ss << "." << static_cast<int>(c[i]);
    }
    return ss.str();
}

Json Adaptor::toJson() const
{
    Json j;
    j["name"] = name;
    j["desc"] = desc;
    j["mac"] = mac.toStr();
    j["ip"] = ip.toStr();
    j["mask"] = mask.toStr();
    j["gateway"] = gateway.toStr();
    return j;
}

Adaptor const& Adaptor::fit(Ip4 const& hint)
{
    auto apts = allAdaptors();
    auto it = std::find_if(apts.begin(), apts.end(), [&](Adaptor const& apt) {
        return apt.mask != Ip4::kNull && apt.gateway != Ip4::kNull &&
               (hint != Ip4::kNull ? apt.ip.onSameLAN(hint, apt.mask) : true);
    });
    if (it == apts.end()) {
        throw std::runtime_error(fmt::format("no local adapter match {}", hint.toStr()));
    }
    return *it;
}

}  // namespace net
