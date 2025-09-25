#include "httpservice/httpclient.h"

namespace lon
{
namespace httpservice
{
HttpResult::Ptr HttpClient::request(http::HttpMethod method, const std::string &url,
                                    uint64_t timeout_ms, const http::HttpRequest::MapType &headers,
                                    const std::string &body)
{
    auto uri = net::Uri::create(url);
    if (!uri)
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::INVALID_URL, nullptr,
                                            "invalid url");
    }
    return request(method, uri, timeout_ms, headers, body);
}

HttpResult::Ptr HttpClient::request(http::HttpMethod method, const net::Uri::Ptr &uri,
                                    uint64_t timeout_ms, const http::HttpRequest::MapType &headers,
                                    const std::string &body)
{
    auto req = std::make_shared<http::HttpRequest>();
    req->setMethod(method);
    req->setPath(uri->getPath());
    req->setFragment(uri->getFragment());
    req->setQuery(uri->getQuery());
    bool has_hosts = false;
    for (auto &header : headers)
    {
        if (util::toLower(header.first) == "connection")
        {
            if (util::toLower(header.second) == "keep-alive")
            {
                req->setClose(false);
            }
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
    req->setBody(body);

    return request(req, uri, timeout_ms);
}

HttpResult::Ptr HttpClient::request(const http::HttpRequest::Ptr &req, const net::Uri::Ptr &uri,
                                    uint64_t timeout_ms)
{
    auto addr = uri->create();
    if (!addr)
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::INVALID_HOST, nullptr,
                                            "invalid host=" + uri->getHost());
    }
    auto socket = net::Socket::create(addr);
    if (LON_UNLIKELY(!socket))
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::SOCKET_FAILED, nullptr,
                                            "socket create failed");
    }
    if (!socket->connect(addr))
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::CONNECTION_FAILED, nullptr,
                                            "socket connect failed, error");
    }
    socket->setRecvTimeout(timeout_ms);
    auto connection = std::make_shared<HttpConnection>(socket);
    auto ret        = connection->sendRequest(req);
    if (ret == 0)
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::SEND_CLOSE_BY_PEER, nullptr,
                                            "send request closed by peer=" + addr->toString());
    }
    else if (ret < 0)
    {
        return std::make_shared<HttpResult>(
            (int32_t)HttpResult::Error::CONNECTION_FAILED, nullptr,
            "send request socket error=" + util::lexical_cast<std::string>(socket->getError()) +
                ", errstr=" + std::string(strerror(socket->getError())));
    }
    auto response = connection->recvResponse();
    if (!response)
    {
        return std::make_shared<HttpResult>(
            (int32_t)HttpResult::Error::RECV_TIMEOUT, nullptr,
            "recv response timeout=" + addr->toString() +
                ", timeout ms=" + util::lexical_cast<std::string>(timeout_ms));
    }
    return std::make_shared<HttpResult>((int32_t)HttpResult::Error::OK, response, "");
}

} // namespace httpservice
} // namespace lon