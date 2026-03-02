#include "websocket/wssession.h"

namespace lon
{
namespace ws
{
static auto g_logger = LON_LOG_ROOT;

WSSession::WSSession(const net::Socket::Ptr &socket, bool proxy, size_t buffer_size)
    : httpservice::HttpSession(socket, proxy, buffer_size)
{
}

http::HttpRequest::Ptr WSSession::handleShake()
{
    http::HttpRequest::Ptr req = nullptr;
    do
    {
        req = recvRequest();
        if (!req)
        {
            LON_INFO(g_logger) << "invalid http request";
            break;
        }
        if (util::toLower(req->getHeader("upgrade")) != "websocket")
        {
            LON_INFO(g_logger) << "http header upgrade != websocket";
            break;
        }
        if (util::toLower(req->getHeader("connection")) != "upgrade")
        {
            LON_INFO(g_logger) << "http header connection != upgrade";
            break;
        }
        if (req->getHeader<int>("sec-websocket-version") != 13)
        {
            LON_INFO(g_logger) << "http header sec-websocket-version != 13";
            break;
        }
        auto key = req->getHeader("sec-websocket-key");
        if (key.empty())
        {
            LON_INFO(g_logger) << "http header sec-websocket-key is empty";
            break;
        }

        std::string val = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        val             = util::HashUtil::base64encode(util::HashUtil::sha1sum(val));

        req->setWebsocket(true);
        auto res = std::make_shared<http::HttpResponse>(req->getVersion(), req->isClose());
        res->setWebsocket(true);
        res->setStatus(http::HttpStatus::SWITCHING_PROTOCOLS);
        res->setHeaders({
            {"Upgrade", "Websocket"},
            {"Connection", "Upgrade"},
            {"Sec-WebSocket-Accept", val},
        });
        sendResponse(res);
        return req;
    } while (false);
    return nullptr;
}

WSFrameMessage::Ptr WSSession::recvMessage() { return WebSocket::recvMessage(this, false); }

int32_t WSSession::sendMessage(const WSFrameMessage::Ptr &msg, bool fin)
{
    return WebSocket::sendMessage(this, msg, false, fin);
}

int32_t WSSession::sendMessage(const std::string &msg, int32_t opcode, bool fin)
{
    return WebSocket::sendMessage(this, std::make_shared<WSFrameMessage>(opcode, msg), false, fin);
}

int32_t WSSession::ping() { return WebSocket::ping(this); }

int32_t WSSession::pong() { return WebSocket::pong(this); }
} // namespace ws
} // namespace lon