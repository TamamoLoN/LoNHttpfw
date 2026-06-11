#pragma once

#include "httpservice/httpconnection.h"
#include "websocket/websocket.h"

namespace lon
{
namespace ws
{
class LON_API WSConnection : public httpservice::HttpConnection
{
  public:
    using Ptr = std::shared_ptr<WSConnection>;
    WSConnection(const net::Socket::Ptr &socket, bool proxy = true, size_t buffer_size = 4096);

    static std::pair<httpservice::HttpResult::Ptr, WSConnection::Ptr>
    create(const std::string &uri, uint64_t timeout_ms,
           const std::map<std::string, std::string> &headers = {});
    static std::pair<httpservice::HttpResult::Ptr, WSConnection::Ptr>
    create(const net::Uri::Ptr &uri, uint64_t timeout_ms,
           const std::map<std::string, std::string> &headers = {});
    WSFrameMessage::Ptr recvMessage();
    int32_t sendMessage(const WSFrameMessage::Ptr &msg, bool fin = true);
    int32_t sendMessage(const std::string &msg, int32_t opcode = WSFrameHead::TEXT_FRAME,
                        bool fin = true);
    int32_t ping();
    int32_t pong();

  private:
};
} // namespace ws
} // namespace lon