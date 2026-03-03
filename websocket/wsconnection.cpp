#include "websocket/wsconnection.h"

namespace lon
{
namespace ws
{
WSConnection::WSConnection(const net::Socket::Ptr &socket, bool proxy, size_t buffer_size)
    : httpservice::HttpConnection(socket, proxy, buffer_size)
{
}

std::pair<httpservice::HttpResult::Ptr, WSConnection::Ptr>
WSConnection::create(const std::string &uri, uint64_t timeout_ms,
                     const std::map<std::string, std::string> &headers)
{
    auto uri_ptr = net::Uri::create(uri);
    if (!uri_ptr)
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::INVALID_URL, nullptr,
                                  "invalid url=" + uri),
                              nullptr);
    }
    return create(uri_ptr, timeout_ms, headers);
}

std::pair<httpservice::HttpResult::Ptr, WSConnection::Ptr>
WSConnection::create(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                     const std::map<std::string, std::string> &headers)
{
    bool is_https = (uri->getScheme() == "https");
    auto addr     = uri->create();
    if (!addr)
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::INVALID_HOST, nullptr,
                                  "invalid host=" + uri->getHost()),
                              nullptr);
    }
    auto socket = is_https ? net::SSLSocket::create(addr) : net::Socket::create(addr);
    if (LON_UNLIKELY(!socket))
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::SOCKET_FAILED, nullptr,
                                  "socket create failed"),
                              nullptr);
    }
    if (!socket->connect(addr))
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::CONNECTION_FAILED,
                                  nullptr, "socket connect failed, error"),
                              nullptr);
    }
    socket->setRecvTimeout(timeout_ms);

    // req
    auto req = std::make_shared<http::HttpRequest>();
    req->setWebsocket(true);
    req->setMethod(http::HttpMethod::GET);
    req->setPath(uri->getPath());
    req->setFragment(uri->getFragment());
    req->setQuery(uri->getQuery());
    bool has_hosts      = false;
    bool has_connection = false;
    for (auto &header : headers)
    {

        if (util::toLower(header.first) == "connection")
        {
            has_connection = true;
            continue;
        }
        if (!has_hosts && util::toLower(header.first) == "host")
        {
            has_hosts = !header.second.empty();
        }
        req->setHeader(header.first, header.second);
    }
    if (!has_hosts)
    {
        req->setHeader("Host", uri->getHost());
    }
    if (!has_connection)
    {
        req->setHeader("Connection", "Upgrade");
    }
    req->setHeader("Upgrade", "WebSocket");
    req->setHeader("Sec-WebSocket-Version", "13");
    req->setHeader("Sec-WebSocket-Key",
                   lon::util::HashUtil::base64encode(lon::util::HashUtil::random_string(16)));

    // connection
    auto connection = std::make_shared<WSConnection>(
        socket, true,
        http::HttpGlobalConfig::Instance().config_http->getData().response.buffer_size);
    auto ret = connection->sendRequest(req);
    if (ret == 0)
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::SEND_CLOSE_BY_PEER,
                                  nullptr, "send request closed by peer=" + addr->toString()),
                              nullptr);
    }
    else if (ret < 0)
    {
        return std::make_pair(
            std::make_shared<httpservice::HttpResult>(
                (int32_t)httpservice::HttpResult::Error::CONNECTION_FAILED, nullptr,
                "send request socket error=" + util::lexical_cast<std::string>(socket->getError()) +
                    ", errstr=" + std::string(strerror(socket->getError()))),
            nullptr);
    }
    auto response = connection->recvResponse();
    if (!response)
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::RECV_TIMEOUT, nullptr,
                                  "recv response timeout=" + addr->toString() + ", timeout ms=" +
                                      util::lexical_cast<std::string>(timeout_ms)),
                              nullptr);
    }
    if (response->getStatus() != http::HttpStatus::SWITCHING_PROTOCOLS)
    {
        return std::make_pair(std::make_shared<httpservice::HttpResult>(
                                  (int32_t)httpservice::HttpResult::Error::CUSTOM, nullptr,
                                  "do not support websocket"),
                              nullptr);
    }
    return std::make_pair(std::make_shared<httpservice::HttpResult>(
                              (int32_t)httpservice::HttpResult::Error::OK, response, ""),
                          connection);
}

WSFrameMessage::Ptr WSConnection::recvMessage() { return WebSocket::recvMessage(this, true); }

int32_t WSConnection::sendMessage(const WSFrameMessage::Ptr &msg, bool fin)
{
    return WebSocket::sendMessage(this, msg, true, fin);
}

int32_t WSConnection::sendMessage(const std::string &msg, int32_t opcode, bool fin)
{
    return WebSocket::sendMessage(this, std::make_shared<WSFrameMessage>(opcode, msg), true, fin);
}

int32_t WSConnection::ping() { return WebSocket::ping(this); }

int32_t WSConnection::pong() { return WebSocket::pong(this); }
} // namespace ws
} // namespace lon