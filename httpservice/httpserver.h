#pragma once

#include "httpservice/httpservlet.h"
#include "httpservice/httpsession.h"
#include "server/serverfactory.h"
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

class HttpServerRegister
{
  public:
    HttpServerRegister(const HttpServerRegister &) = delete;
    HttpServerRegister &operator=(const HttpServerRegister &) = delete;
    HttpServerRegister(HttpServerRegister &&)                 = delete;
    HttpServerRegister &operator=(HttpServerRegister &&) = delete;
    static HttpServerRegister &Instance();

  private:
    HttpServerRegister();
};
#define REGISTER_HTTPSERVER                                                                        \
    auto &_REGISTER_HTTPSERVER = lon::httpservice::HttpServerRegister::Instance();
} // namespace httpservice
} // namespace lon