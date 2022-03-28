// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_NET_ADDRESS_HPP
#define DMITIGR_NET_ADDRESS_HPP

#include "../base/assert.hpp"
#include "../fs/filesystem.hpp"
#include "exceptions.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

#ifdef _WIN32
#include "../os/windows.hpp"

#include <Winsock2.h> // includes Ws2def.h
#include <In6addr.h>  // must follows after Winsock2.h
#include <Ws2tcpip.h> // inet_pton(), inet_ntop()
#include <afunix.h>   // AF_UNIX, sockaddr_un
#else
#include <arpa/inet.h>
#include <sys/un.h>
#endif

namespace dmitigr::net {

/// A protocol family.
enum class Protocol_family {
  /// Local communication.
  local = AF_UNIX,

  /// The IP version 4 Internet protocols.
  ipv4 = AF_INET,

  /// The IP version 6 Internet protocols.
  ipv6 = AF_INET6
};

/// @returns Native version of `value`.
inline int to_native(const Protocol_family value)
{
  return static_cast<int>(value);
}

/// An IP address.
class Ip_address final {
public:
  /// Constructs invalid instance.
  Ip_address() noexcept
  {
    DMITIGR_ASSERT(!is_valid());
  }

  /// @returns A new instance from text representation.
  static Ip_address from_text(const std::string& str)
  {
    Ip_address result;
    char buf[sizeof(::in6_addr)];
    for (const auto fam : {AF_INET, AF_INET6}) {
      if (const int r = inet_pton__(fam, str.c_str(), buf)) {
        if (r > 0) {
          if (fam == AF_INET)
            result.init< ::in_addr>({buf, sizeof(::in_addr)});
          else
            result.init< ::in6_addr>({buf, sizeof(::in6_addr)});
          DMITIGR_ASSERT(result.is_valid());
          break; // done
        }
      }
    }
    return result;
  }

  /// @returns A new instance from binary representation.
  static Ip_address from_binary(const std::string_view bin)
  {
    Ip_address result;
    const auto binsz = bin.size();
    if (binsz == 4)
      result.init< ::in_addr>(bin);
    else if (binsz == 16)
      result.init< ::in6_addr>(bin);
    return result;
  }

  /// @returns `true` if this instance is valid.
  bool is_valid() const noexcept
  {
    return is_valid_;
  }

  /// @returns `true` if the instance is valid.
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /// @returns `true` if `text` is either valid IPv4 or IPv6 address.
  static bool is_valid(const std::string& str)
  {
    char buf[sizeof(::in6_addr)];
    return inet_pton__(AF_INET, str.c_str(), buf) > 0 ||
      inet_pton__(AF_INET6, str.c_str(), buf) > 0;
  }

  /// @returns The family of the IP address, or undefined value if `!is_valid()`.
  Protocol_family family() const noexcept
  {
    if (!is_valid())
      return Protocol_family{AF_UNSPEC};

    return std::visit([](const auto& addr) {
      using T = std::decay_t<decltype (addr)>;
      static_assert(std::is_same_v<T, ::in_addr> ||
        std::is_same_v<T, ::in6_addr>);
      if constexpr (std::is_same_v<T, ::in_addr>) {
        return Protocol_family::ipv4;
      } else {
        return Protocol_family::ipv6;
      }
    }, binary_);
  }

  /**
   * @returns The binary representation (network byte ordering) of the IP
   * address, or `nullptr` if `!is_valid()`.
   */
  const void* binary() const noexcept
  {
    if (!is_valid())
      return nullptr;

    return std::visit([](const auto& addr) {
      const void* const result = &addr;
      return result;
    }, binary_);
  }

  /**
   * @returns The result of conversion of this IP address to the instance of
   * type `std::string`, or empty string if `!is_valid()`.
   */
  std::string to_string() const
  {
    if (!is_valid())
      return std::string{};

    const auto fam = to_native(family());
    const std::string::size_type result_max_size = (fam == AF_INET) ? 16 : 46;
    std::string result(result_max_size, '\0');
    inet_ntop__(fam, binary(), result.data(),
      static_cast<unsigned>(result.size()));

    // Trimming right zeros.
    const auto b = cbegin(result);
    const auto i = std::find(b, cend(result), '\0');
    result.resize(static_cast<std::string::size_type>(i - b));

    return result;
  }

private:
  bool is_valid_{};
  std::variant<::in_addr, ::in6_addr> binary_;

  void* binary__()
  {
    return const_cast<void*>(static_cast<const Ip_address*>(this)->binary());
  }

  template<typename Addr>
  void init(const std::string_view bin)
  {
    DMITIGR_ASSERT(sizeof(Addr) == bin.size());
    Addr tmp;
    std::memcpy(&tmp, bin.data(), bin.size());
    binary_ = tmp;
    is_valid_ = true;
  }

  /**
   * @brief Converts text representation of IP address to numeric representation.
   *
   * @returns The value greater or equal to zero.
   */
  static int inet_pton__(const int af, const char* const src, void* const dst)
  {
    DMITIGR_ASSERT((af == AF_INET || af == AF_INET6) && src && dst);
    const int result = ::inet_pton(af, src, dst);
    if (result < 0)
      throw DMITIGR_NET_EXCEPTION{"cannot convert text "
        "representation of IP address to numeric representation"};
    return result;
  }

  /**
   * @brief Converts numeric representation of IP address to text representation.
   *
   * @param[out] dst The result parameter.
   */
  static void inet_ntop__(const int af, const void* const src, char* const dst,
    const unsigned dst_size)
  {
    DMITIGR_ASSERT((af == AF_INET || af == AF_INET6) && src && dst &&
      dst_size >= 16);
    if (!::inet_ntop(af, src, dst, dst_size))
      throw DMITIGR_NET_EXCEPTION{"cannot convert numeric "
        "representation of IP address to text representation"};
  }
};

/// An socket address.
class Socket_address final {
public:
  /// Constructs TCP socket address.
  Socket_address(const Ip_address& ip, const int port)
  {
    if (ip.family() == Protocol_family::ipv4) {
      ::sockaddr_in addr{};
      constexpr auto addr_size = sizeof(addr);
      std::memset(&addr, 0, addr_size);
      addr.sin_family = AF_INET;
      addr.sin_addr = *static_cast<const ::in_addr*>(ip.binary());
      addr.sin_port = htons(static_cast<unsigned short>(port));
      binary_ = addr;
    } else if (ip.family() == Protocol_family::ipv6) {
      ::sockaddr_in6 addr{};
      constexpr auto addr_size = sizeof(addr);
      std::memset(&addr, 0, addr_size);
      addr.sin6_family = AF_INET6;
      addr.sin6_addr = *static_cast<const ::in6_addr*>(ip.binary());
      addr.sin6_port = htons(static_cast<unsigned short>(port));
      addr.sin6_flowinfo = htonl(0);
      addr.sin6_scope_id = htonl(0);
      binary_ = addr;
    }
  }

  /// Constructs UDS address.
  Socket_address(const std::filesystem::path& path)
  {
    ::sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    constexpr auto max_path_size = sizeof(::sockaddr_un::sun_path) - 1;
    if (path.native().size() <= max_path_size) {
      const auto pathstr = path.string();
#ifdef _WIN32
      const auto err = ::strncpy_s(addr.sun_path, max_path_size + 1,
        pathstr.c_str(), pathstr.size());
      DMITIGR_ASSERT(!err);
#else
      std::strncpy(addr.sun_path, pathstr.c_str(), max_path_size);
      addr.sun_path[max_path_size] = '\0';
#endif
    } else
      throw Exception{"UDS path too long"};
    binary_ = addr;
  }

  /// @returns The family of the socket address.
  Protocol_family family() const
  {
    return std::visit([](const auto& addr) {
      using T = std::decay_t<decltype (addr)>;
      static_assert(std::is_same_v<T, ::sockaddr_un> ||
        std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>);
      if constexpr (std::is_same_v<T, ::sockaddr_un>) {
        return Protocol_family::local;
      } else if constexpr (std::is_same_v<T, ::sockaddr_in>) {
        return Protocol_family::ipv4;
      } else {
        return Protocol_family::ipv6;
      }
    }, binary_);
  }

  /// @returns The binary representation of the socket address.
  const void* binary() const
  {
    return std::visit([](auto& addr) {
      const void* const result = &addr;
      return result;
    }, binary_);
  }

  /// @returns The sockaddr representation of the socket address.
  const sockaddr* addr() const
  {
    return static_cast<const sockaddr*>(binary());
  }

  /// @returns The size of underlying binary data.
  unsigned size() const
  {
    return std::visit([](const auto& addr) {
      constexpr auto sz = sizeof(std::decay_t<decltype(addr)>);
      static_assert(sz <= std::numeric_limits<unsigned>::max());
      return static_cast<unsigned>(sz);
    }, binary_);
  }

private:
  std::variant<::sockaddr_un, ::sockaddr_in, ::sockaddr_in6> binary_;
};

} // namespace dmitigr::net

#endif  // DMITIGR_NET_ADDRESS_HPP
