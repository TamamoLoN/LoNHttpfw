#pragma once

#include "server/tcpserver.h"
#include "websocket/wsservlet.h"

namespace lon
{
namespace ws
{
class WSServer : public server::TcpServer
{
  public:
    using Ptr = std::shared_ptr<WSServer>;

    WSServer(scheduler::IOScheduler *scheduler        = scheduler::IOScheduler::getThis(),
             scheduler::IOScheduler *accept_scheduler = scheduler::IOScheduler::getThis(),
             size_t client_timeout = 1000 * 60 * 2, const std::string &name = "UNKNOWN");
    void handleClient(const net::Socket::Ptr &client) override;

    void setDispatch(const WSServletDispatch::Ptr &dispatch);
    WSServletDispatch::Ptr getDispatch();

  private:
    WSServletDispatch::Ptr m_dispatch;
};
} // namespace ws
} // namespace lon