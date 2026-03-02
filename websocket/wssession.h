#pragma once

#include "httpservice/httpsession.h"
#include "websocket/websocket.h"

namespace lon
{
namespace ws
{
class WSSession : public httpservice::HttpSession
{
  public:
    using Ptr = std::shared_ptr<WSSession>;
    WSSession(const net::Socket::Ptr &socket, bool proxy = true, size_t buffer_size = 4096);

    /// server client
    http::HttpRequest::Ptr handleShake();

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