#include "httpservice/httpserver.h"

namespace lon
{
namespace httpservice
{
static auto g_logger = LON_LOG_ROOT;

HttpServer::HttpServer(scheduler::IOScheduler *scheduler, scheduler::IOScheduler *accept_scheduler,
                       size_t client_timeout, const std::string &name, bool keepalive)
    : TcpServer(scheduler, accept_scheduler, client_timeout, name), m_keepalive(keepalive),
      m_dispatch(std::make_shared<HttpServletDispatch>())
{
}

void HttpServer::handleClient(const net::Socket::Ptr &client)
{
    LON_INFO(g_logger) << "[" << getName() << "] client connected, client=" << client->toString();
    auto session = std::make_shared<HttpSession>(
        client, true,
        http::HttpGlobalConfig::Instance().config_http->getData().request.buffer_size);

    do
    {
        auto request = session->recvRequest();
        if (!request)
        {
            if (!session->isEof())
            {
                LON_ERROR(g_logger)
                    << "[" << getName() << "] recv http request failed, errno=" << errno
                    << ", errmsg=" << strerror(errno) << ", client=" << client->toString();
            }
            break;
        }
        auto response = std::make_shared<http::HttpResponse>(request->getVersion(),
                                                             request->isClose() || !m_keepalive);
        response->setHeader("Server", getName());
        if (m_dispatch->handle(request, response, session) == -1)
        {
            m_dispatch->getDefaultServlet()->handle(request, response, session);
        }
        // LON_DEBUG(g_logger) << "[" << getName() << "] recv http request =" << *request;
        // LON_DEBUG(g_logger) << "[" << getName() << "] send http response=" << *response;
        session->sendResponse(response);
        if (response->isClose())
        {
            break;
        }
    } while (m_keepalive);
    LON_INFO(g_logger) << "[" << getName()
                       << "] client disconnected, client=" << client->toString();
    session->close();
}

void HttpServer::setDispatch(const HttpServletDispatch::Ptr &dispatch) { m_dispatch = dispatch; }

HttpServletDispatch::Ptr HttpServer::getDispatch() { return m_dispatch; }

} // namespace httpservice
} // namespace lon