#include "type.h"
#include "platform.h"
#include <sstream>
#include <regex>
#include <boost/algorithm/string.hpp>

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

Ip4 Ip4::fromDottedDec(std::string const& s)
{
    static std::regex pat(R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$)");
    if (!regex_match(s, pat)) {
        THROW_(fmt::format("invalid ip4: {}", s));
    }
    std::vector<std::string> parts;
    boost::split(parts, s, boost::is_any_of("."));
    Ip4 ip;
    ip.b1 = std::stoi(parts[0]);
    ip.b2 = std::stoi(parts[1]);
    ip.b3 = std::stoi(parts[2]);
    ip.b4 = std::stoi(parts[3]);
    return ip;
}

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

bool Ip4::operator<(Ip4 const& rhs) const
{
    auto i = reinterpret_cast<uint32_t const*>(this);
    auto j = reinterpret_cast<uint32_t const*>(&rhs);
    return *i < *j;
}

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
    auto& apts = allAdaptors();
    auto it = std::find_if(apts.begin(), apts.end(), [&](Adaptor const& apt) {
        return apt.mask != Ip4::kNull && apt.gateway != Ip4::kNull &&
               (hint != Ip4::kNull ? apt.ip.onSameLAN(hint, apt.mask) : true);
    });
    if (it == apts.end()) {
        THROW_(fmt::format("no local adapter match {}", hint.toStr()));
    }
    return *it;
}

BytesReader::BytesReader(std::vector<uint8_t> const& bytes)
    : data_(bytes.data()), size_(bytes.size())
{
}

#define CHECK_SIZE(n)                                                                 \
    if (size_ < n) {                                                                  \
        THROW_(fmt::format("inadequate bytes: {} expected, only {} left", n, size_)); \
    }

uint8_t BytesReader::read8u()
{
    CHECK_SIZE(sizeof(uint8_t));
    uint8_t b = *reinterpret_cast<uint8_t const*>(data_);
    data_ += sizeof(uint8_t);
    size_ -= sizeof(uint8_t);
    return b;
}

uint16_t BytesReader::read16u(bool ntoh)
{
    CHECK_SIZE(sizeof(uint16_t));
    uint16_t b = *reinterpret_cast<uint16_t const*>(data_);
    data_ += sizeof(uint16_t);
    size_ -= sizeof(uint16_t);
    return ntoh ? ntohs(b) : b;
}

uint32_t BytesReader::read32u(bool ntoh)
{
    CHECK_SIZE(sizeof(uint32_t));
    uint32_t b = *reinterpret_cast<uint32_t const*>(data_);
    data_ += sizeof(uint32_t);
    size_ -= sizeof(uint32_t);
    return ntoh ? ntohl(b) : b;
}

Ip4 BytesReader::readIp4()
{
    CHECK_SIZE(sizeof(Ip4));
    Ip4 b = *reinterpret_cast<Ip4 const*>(data_);
    data_ += sizeof(Ip4);
    size_ -= sizeof(Ip4);
    return b;
}

Mac BytesReader::readMac()
{
    CHECK_SIZE(sizeof(Mac));
    Mac b = *reinterpret_cast<Mac const*>(data_);
    data_ += sizeof(Mac);
    size_ -= sizeof(Mac);
    return b;
}

std::vector<uint8_t> BytesReader::readBytes(size_t n)
{
    CHECK_SIZE(n);
    std::vector<uint8_t> b(data_, data_ + n);
    data_ += n;
    size_ -= n;
    return b;
}

std::vector<uint8_t> BytesReader::readAll()
{
    std::vector<uint8_t> b(data_, data_ + size_);
    data_ += size_;
    size_ = 0;
    return b;
}

void BytesBuilder::prependTo(std::vector<uint8_t>& bytes)
{
    bytes.insert(bytes.begin(), data_.begin(), data_.end());
}

void BytesBuilder::write8u(uint8_t b)
{
    uint8_t* d = reinterpret_cast<uint8_t*>(&b);
    data_.insert(data_.end(), d, d + sizeof(uint8_t));
}

void BytesBuilder::write16u(uint16_t b, bool hton)
{
    uint16_t h = hton ? htons(b) : b;
    uint8_t* d = reinterpret_cast<uint8_t*>(&h);
    data_.insert(data_.end(), d, d + sizeof(uint16_t));
}

void BytesBuilder::write32u(uint32_t b, bool hton)
{
    uint32_t h = hton ? htonl(b) : b;
    uint8_t* d = reinterpret_cast<uint8_t*>(&h);
    data_.insert(data_.end(), d, d + sizeof(uint32_t));
}

void BytesBuilder::writeIp4(Ip4 b)
{
    uint8_t* d = reinterpret_cast<uint8_t*>(&b);
    data_.insert(data_.end(), d, d + sizeof(Ip4));
}

void BytesBuilder::writeMac(Mac const& b)
{
    uint8_t const* d = reinterpret_cast<uint8_t const*>(&b);
    data_.insert(data_.end(), d, d + sizeof(Mac));
}

void BytesBuilder::writeBytes(uint8_t const* b, size_t size)
{
    data_.insert(data_.end(), b, b + size);
}

void BytesBuilder::writeBytes(std::vector<uint8_t> const& b) { writeBytes(b.data(), b.size()); }

}  // namespace net
