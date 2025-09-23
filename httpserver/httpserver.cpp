#include "httpserver/httpserver.h"

namespace lon
{
namespace httpserver
{
HttpServer::HttpServer(scheduler::IOScheduler *scheduler, scheduler::IOScheduler *accept_scheduler,
                       size_t client_timeout, const std::string &name, bool keepalive)
    : TcpServer(scheduler, accept_scheduler, client_timeout, name), m_keepalive(keepalive)
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
        response->setBody("hello lon\n");
        LON_DEBUG(LON_LOG_ROOT) << "[" << getName() << "] recv http request =" << *request;
        LON_DEBUG(LON_LOG_ROOT) << "[" << getName() << "] send http response=" << *response;
        session->sendResponse(response);
    } while (m_keepalive);

    session->close();
}
} // namespace httpserver
} // namespace lon