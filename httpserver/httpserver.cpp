#include "httpserver/httpserver.h"

namespace lon
{
namespace httpserver
{
HttpServer::HttpServer(scheduler::IOScheduler *scheduler, scheduler::IOScheduler *accept_scheduler,
                       size_t client_timeout, const std::string &name, bool keepalive)
    : TcpServer(scheduler, accept_scheduler, client_timeout, name), m_keepalive(keepalive),
      m_dispatch(std::make_shared<HttpServletDispatch>())
{
}

void HttpServer::handleClient(const net::Socket::Ptr &client)
{
    LON_INFO(LON_LOG_ROOT) << "[" << getName() << "] handle client, client=" << client->toString();
    auto session = std::make_shared<HttpSession>(
        client, true,
        http::HttpGlobalConfig::Instance().config_http->getData().request.buffer_size);

    do
    {
        auto request = session->recvRequest();
        if (!request)
        {
            LON_ERROR(LON_LOG_ROOT)
                << "[" << getName() << "] recv http request failed, errno=" << errno
                << ", errmsg=" << strerror(errno) << ", client=" << client->toString();
            break;
        }
        auto response = std::make_shared<http::HttpResponse>(request->getVersion(),
                                                             request->isClose() || !m_keepalive);
        m_dispatch->handle(request, response, session);
        LON_DEBUG(LON_LOG_ROOT) << "[" << getName() << "] recv http request =" << *request;
        LON_DEBUG(LON_LOG_ROOT) << "[" << getName() << "] send http response=" << *response;
        session->sendResponse(response);
    } while (m_keepalive);

    session->close();
}

void HttpServer::setDispatch(const HttpServletDispatch::Ptr &dispatch) { m_dispatch = dispatch; }

HttpServletDispatch::Ptr HttpServer::getDispatch() { return m_dispatch; }

} // namespace httpserver
} // namespace lon