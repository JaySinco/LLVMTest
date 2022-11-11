#include "platform.h"
#define UNICODE
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

namespace net
{

struct WSAGuard
{
    WSAGuard()
    {
        WSADATA ws;
        WSAStartup(MAKEWORD(2, 2), &ws);
    }

    ~WSAGuard() { WSACleanup(); }

private:
    static WSAGuard g;
};

WSAGuard WSAGuard::g;

uint32_t htonl(uint32_t hostlong) { return ::htonl(hostlong); }

uint16_t htons(uint16_t hostshort) { return ::htons(hostshort); }

uint32_t ntohl(uint32_t netlong) { return ::ntohl(netlong); }

uint16_t ntohs(uint16_t netshort) { return ::ntohs(netshort); }

static Ip4 ip4FromInAddr(in_addr const& addr)
{
    Ip4 ip;
    ip.b1 = addr.S_un.S_un_b.s_b1;
    ip.b2 = addr.S_un.S_un_b.s_b2;
    ip.b3 = addr.S_un.S_un_b.s_b3;
    ip.b4 = addr.S_un.S_un_b.s_b4;
    return ip;
}

static in_addr ip4ToInAddr(Ip4 const ip)
{
    in_addr addr;
    addr.S_un.S_un_b.s_b1 = ip.b1;
    addr.S_un.S_un_b.s_b2 = ip.b2;
    addr.S_un.S_un_b.s_b3 = ip.b3;
    addr.S_un.S_un_b.s_b4 = ip.b4;
    return addr;
}

Ip4 ip4FromDottedDec(std::string const& s)
{
    in_addr addr;
    if (inet_pton(AF_INET, s.c_str(), &addr) != 1) {
        throw std::runtime_error("failed to call inet_pton");
    }
    return ip4FromInAddr(addr);
}

Ip4 ip4FromDomain(std::string const& s)
{
    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* first_addr;
    auto ret = GetAddrInfoA(s.c_str(), nullptr, &hints, &first_addr);
    if (ret != 0 || first_addr == nullptr) {
        throw std::runtime_error("failed to get addr info");
    }
    auto ip = ip4FromInAddr(reinterpret_cast<sockaddr_in*>(first_addr->ai_addr)->sin_addr);
    freeaddrinfo(first_addr);
    return ip;
}

std::vector<Adaptor> const& allAdaptors()
{
    static std::once_flag flag;
    static std::vector<Adaptor> adapters;

    std::call_once(flag, [&] {
        ULONG buflen = sizeof(IP_ADAPTER_INFO);
        auto plist = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
        auto plist_guard = utils::scopeExit([&]() { free(plist); });
        if (GetAdaptersInfo(plist, &buflen) == ERROR_BUFFER_OVERFLOW) {
            plist = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));
            if (GetAdaptersInfo(plist, &buflen) != NO_ERROR) {
                throw std::runtime_error("failed to get adapters info");
            }
        }
        PIP_ADAPTER_INFO pinfo = plist;
        while (pinfo) {
            Adaptor apt;
            Ip4 ip = ip4FromDottedDec(pinfo->IpAddressList.IpAddress.String);
            if (ip != Ip4::kZeros) {
                apt.name = std::string("\\Device\\NPF_") + pinfo->AdapterName;
                apt.desc = pinfo->Description;
                apt.ip = ip;
                apt.mask = ip4FromDottedDec(pinfo->IpAddressList.IpMask.String);
                apt.gateway = ip4FromDottedDec(pinfo->GatewayList.IpAddress.String);
                if (pinfo->AddressLength != sizeof(Mac)) {
                    spdlog::warn("incompatible mac length: {}", pinfo->AddressLength);
                    continue;
                }
                auto c = reinterpret_cast<uint8_t*>(&apt.mac);
                for (unsigned i = 0; i < pinfo->AddressLength; ++i) {
                    c[i] = pinfo->Address[i];
                }
                adapters.push_back(apt);
            }
            pinfo = pinfo->Next;
        }
        if (adapters.size() <= 0) {
            throw std::runtime_error("failed to find any suitable adapter");
        }
    });

    return adapters;
}

}  // namespace net