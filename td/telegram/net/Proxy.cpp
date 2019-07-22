//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2019
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/telegram/net/Proxy.h"

#include "td/telegram/td_api.h"

namespace td {

Result<Proxy> Proxy::from_td_api(string server, int port, td_api::ProxyType *proxy_type) {
  if (proxy_type == nullptr) {
    return Status::Error(400, "Proxy type should not be empty");
  }
  if (server.empty()) {
    return Status::Error(400, "Server name can't be empty");
  }
  if (port <= 0 || port > 65535) {
    return Status::Error(400, "Wrong port number");
  }

  switch (proxy_type->get_id()) {
    case td_api::proxyTypeSocks5::ID: {
      auto type = static_cast<td_api::proxyTypeSocks5 *>(proxy_type);
      return Proxy::socks5(std::move(server), port, std::move(type->username_), std::move(type->password_));
    }
    case td_api::proxyTypeHttp::ID: {
      auto type = static_cast<td_api::proxyTypeHttp *>(proxy_type);
      if (type->http_only_) {
        return Proxy::http_caching(std::move(server), port, std::move(type->username_), std::move(type->password_));
      } else {
        return Proxy::http_tcp(std::move(server), port, std::move(type->username_), std::move(type->password_));
      }
    }
    case td_api::proxyTypeMtproto::ID: {
      auto type = static_cast<td_api::proxyTypeMtproto *>(proxy_type);
      TRY_RESULT(secret, mtproto::ProxySecret::from_link(type->secret_));
      return Proxy::mtproto(std::move(server), port, std::move(secret));
    }
    default:
      UNREACHABLE();
      return Status::Error(400, "Wrong proxy type");
  }
}

StringBuilder &operator<<(StringBuilder &string_builder, const Proxy &proxy) {
  switch (proxy.type()) {
    case Proxy::Type::Socks5:
      return string_builder << "ProxySocks5 " << proxy.server() << ":" << proxy.port();
    case Proxy::Type::HttpTcp:
      return string_builder << "ProxyHttpTcp " << proxy.server() << ":" << proxy.port();
    case Proxy::Type::HttpCaching:
      return string_builder << "ProxyHttpCaching " << proxy.server() << ":" << proxy.port();
    case Proxy::Type::Mtproto:
      return string_builder << "ProxyMtproto " << proxy.server() << ":" << proxy.port() << "/"
                            << proxy.secret().get_encoded_secret();
    case Proxy::Type::None:
      return string_builder << "ProxyEmpty";
    default:
      UNREACHABLE();
      return string_builder;
  }
}

}  // namespace td
