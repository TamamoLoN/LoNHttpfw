#pragma once

#include "httpservice/httpservlet.h"
#include "httpservice/httpsession.h"
#include "server/tcpserver.h"

namespace lon
{
namespace httpservice
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

    void setDispatch(const HttpServletDispatch::Ptr &dispatch);
    HttpServletDispatch::Ptr getDispatch();

  private:
    bool m_keepalive;
    HttpServletDispatch::Ptr m_dispatch;
};
} // namespace httpservice
} // namespace lon