#include "httpservice/httpconnection.h"

namespace lon
{
namespace httpservice
{
HttpConnection::HttpConnection(const net::Socket::Ptr &socket, bool proxy, size_t buffer_size)
    : net::SocketStream(socket, proxy), m_buffer_size(buffer_size)
{
}

HttpConnection::~HttpConnection() {}

http::HttpResponse::Ptr HttpConnection::recvResponse()
{
    auto parser      = std::make_shared<http::HttpResponseParser>();
    auto buffer_size = m_buffer_size;
    std::shared_ptr<char> buffer(new char[buffer_size + 1](), [](char *ptr) {
        delete[] ptr;
        ptr = nullptr;
    });
    auto data      = buffer.get();
    ssize_t offset = 0;

    do
    {
        ssize_t len = read(data + offset, buffer_size - offset);
        if (len <= 0)
        {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';

        size_t nparse = parser->execute(data, len, false);
        if (parser->error())
        {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == (ssize_t)buffer_size)
        {
            close();
            return nullptr;
        }
        if (parser->finished())
        {
            break;
        }
    } while (true);

    auto &http_parser = parser->getParser();
    std::string body;

    if (http_parser.chunked)
    {
        ssize_t len = offset;
        do
        {
            bool begin = true;
            do
            {
                if (!begin || len == 0)
                {
                    auto rt = read(data + len, buffer_size - len);
                    if (rt <= 0)
                    {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len]     = '\0';
                size_t nparse = parser->execute(data, len, true);
                if (parser->error())
                {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if (len == (ssize_t)buffer_size)
                {
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->finished());

            if (http_parser.content_len + 2 <= len)
            {
                body.append(data, http_parser.content_len);
                memmove(data, data + http_parser.content_len + 2,
                        len - http_parser.content_len - 2);
                len -= http_parser.content_len + 2;
            }
            else
            {
                body.append(data, len);
                int left = http_parser.content_len - len + 2;
                while (left > 0)
                {
                    int rt = read(data, left > (int)buffer_size ? (int)buffer_size : left);
                    if (rt <= 0)
                    {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2); // 去掉最后的 CRLF
                len = 0;
            }
        } while (!http_parser.chunks_done);
    }
    else
    {
        int64_t length = parser->getContentLength();
        if (length > 0)
        {
            body.resize(length);
            ssize_t len = 0;
            if (length >= offset)
            {
                memcpy(&body[0], data, offset);
                len = offset;
            }
            else
            {
                memcpy(&body[0], data, length);
                len = length;
            }
            length -= offset;
            if (length > 0)
            {
                if (readF(&body[len], length) <= 0)
                {
                    close();
                    return nullptr;
                }
            }
        }
    }

    // 设置 body
    parser->getData()->setBody(body);
    return std::static_pointer_cast<http::HttpResponse>(parser->getData());
}

ssize_t HttpConnection::sendRequest(const http::HttpRequest::Ptr &request)
{
    std::stringstream ss;
    ss << *request;
    auto str = ss.str();
    return writeF(str.c_str(), str.size());
}

HttpResult::Ptr HttpConnection::request(http::HttpMethod method, const std::string &url,
                                        uint64_t timeout_ms,
                                        const http::HttpRequest::MapType &headers,
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

HttpResult::Ptr HttpConnection::request(http::HttpMethod method, const net::Uri::Ptr &uri,
                                        uint64_t timeout_ms,
                                        const http::HttpRequest::MapType &headers,
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

HttpResult::Ptr HttpConnection::request(const http::HttpRequest::Ptr &req, const net::Uri::Ptr &uri,
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