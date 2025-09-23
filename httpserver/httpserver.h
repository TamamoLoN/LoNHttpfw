#pragma once

#include "httpserver/httpsession.h"
#include "server/tcpserver.h"

namespace lon
{
namespace httpserver
{
class HttpServer : public server::TcpServer
{
  public:
    using Ptr = std::shared_ptr<HttpServer>;
    HttpServer(scheduler::IOScheduler *scheduler        = scheduler::IOScheduler::getThis(),
               scheduler::IOScheduler *accept_scheduler = scheduler::IOScheduler::getThis(),
               size_t client_timeout = 1000 * 60 * 2, const std::string &name = "UNKNOWN",
               bool keepalive = false);
    void handleClient(const net::Socket::Ptr &client) override;

  private:
    bool m_keepalive;
};
} // namespace httpserver
} // namespace lon