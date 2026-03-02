#include "websocket/wsserver.h"

namespace lon
{
namespace ws
{
static auto g_logger = LON_LOG_ROOT;

WSServer::WSServer(scheduler::IOScheduler *scheduler, scheduler::IOScheduler *accept_scheduler,
                   size_t client_timeout, const std::string &name)
    : TcpServer(scheduler, accept_scheduler, client_timeout, name),
      m_dispatch(std::make_shared<WSServletDispatch>())
{
}

void WSServer::handleClient(const net::Socket::Ptr &client)
{
    LON_INFO(g_logger) << "[" << getName() << "] client connected, client=" << client->toString();
    auto session = std::make_shared<WSSession>(
        client, true,
        http::HttpGlobalConfig::Instance().config_http->getData().request.buffer_size);

    do
    {
        auto request = session->handleShake();
        if (!request)
        {
            if (!session->isEof())
            {
                LON_ERROR(g_logger)
                    << "[" << getName() << "] handle shake failed, errno=" << errno
                    << ", errmsg=" << strerror(errno) << ", client=" << client->toString();
            }
            break;
        }
        auto servlet = m_dispatch->getWSServlet(request->getPath());
        if (!servlet)
        {
            LON_ERROR(g_logger) << "[" << getName()
                                << "] servlet not match, path=" << request->getPath();
            break;
        }
        auto ret = servlet->onConnect(request, session);
        if (ret)
        {
            LON_ERROR(g_logger) << "[" << getName() << "] on connect failed, ret=" << ret;
            break;
        }
        while (true)
        {
            auto msg = session->recvMessage();
            if (!msg)
            {
                break;
            }
            ret = servlet->handle(request, msg, session);
            if (ret)
            {
                LON_ERROR(g_logger) << "[" << getName() << "] handle failed, ret=" << ret;
                break;
            }
        }
        servlet->onClose(request, session);
    } while (false);
    LON_INFO(g_logger) << "[" << getName()
                       << "] client disconnected, client=" << client->toString();
    session->close();
}

void WSServer::setDispatch(const WSServletDispatch::Ptr &dispatch) { m_dispatch = dispatch; }

WSServletDispatch::Ptr WSServer::getDispatch() { return m_dispatch; }
} // namespace ws
} // namespace lon