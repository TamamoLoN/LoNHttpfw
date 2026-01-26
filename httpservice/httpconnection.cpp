#include "httpservice/httpconnection.h"

namespace lon
{
namespace httpservice
{
static auto g_logger = LON_LOG_ROOT;

HttpConnection::HttpConnection(const net::Socket::Ptr &socket, bool proxy, size_t buffer_size)
    : net::SocketStream(socket, proxy), m_buffer_size(buffer_size),
      m_create_time_ms(util::getCurrentMs()), m_request_count(0)
{
}

HttpConnection::~HttpConnection() {}

const uint64_t HttpConnection::getCreateTimeMs() const { return m_create_time_ms; }

uint64_t &HttpConnection::getRequestCount() { return m_request_count; }

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

HttpResult::Ptr HttpConnection::get(const std::string &url, uint64_t timeout_ms,
                                    const http::HttpRequest::MapType &headers,
                                    const std::string &body)
{
    return request(http::HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnection::get(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                                    const http::HttpRequest::MapType &headers,
                                    const std::string &body)
{
    return request(http::HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnection::post(const std::string &url, uint64_t timeout_ms,
                                     const http::HttpRequest::MapType &headers,
                                     const std::string &body)
{
    return request(http::HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnection::post(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                                     const http::HttpRequest::MapType &headers,
                                     const std::string &body)
{
    return request(http::HttpMethod::POST, uri, timeout_ms, headers, body);
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
    auto connection = std::make_shared<HttpConnection>(
        socket, true,
        http::HttpGlobalConfig::Instance().config_http->getData().response.buffer_size);
    auto ret = connection->sendRequest(req);
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

HttpConnectionPool::HttpConnectionPool(const std::string &host, const std::string &vhost,
                                       in_port_t port, uint32_t max_size, uint32_t max_alive_time,
                                       uint32_t max_request_count, bool keepalive,
                                       size_t buffer_size)
    : m_host(host), m_vhost(vhost), m_port(port), m_max_size(max_size),
      m_max_alive_time(max_alive_time), m_max_request_count(max_request_count),
      m_keepalive(keepalive), m_buffer_size(buffer_size), m_connections({}), m_size({0})
{
}

HttpConnectionPool::~HttpConnectionPool()
{
    MutexType::Lock lock(m_mutex);
    for (auto &it : m_connections)
    {
        delete it;
        it = nullptr;
    }
}

HttpConnection::Ptr HttpConnectionPool::getConnection()
{
    auto now_ms = util::getCurrentMs();
    std::vector<HttpConnection *> invalid_connections;
    HttpConnection *res = nullptr;
    MutexType::Lock lock(m_mutex);

    while (!m_connections.empty())
    {
        auto connection = *m_connections.begin();
        m_connections.pop_front();
        if (!connection->isConnected())
        {
            invalid_connections.push_back(connection);
            continue;
        }
        if ((connection->getCreateTimeMs() + m_max_alive_time) < now_ms)
        {
            invalid_connections.push_back(connection);
            continue;
        }
        res = connection;
        break;
    }
    lock.unlock();
    for (auto &it : invalid_connections)
    {
        delete it;
        it = nullptr;
    }
    m_size -= invalid_connections.size();
    if (!res)
    {
        net::IPAddress::Ptr addr = nullptr;
        bool ret                 = net::Address::parseIPAddress(addr, m_host);
        if (LON_UNLIKELY(!ret))
        {
            LON_ERROR(g_logger) << "parse host=" << m_host << " failed";
            return nullptr;
        }
        if (!addr->getPort())
        {
            addr->setPort(m_port);
        }
        auto socket = net::Socket::create(addr);
        if (LON_UNLIKELY(!socket))
        {
            LON_ERROR(g_logger) << "create socket failed, addr=" << addr->toString();
            return nullptr;
        }
        if (LON_UNLIKELY(!socket->connect(addr)))
        {
            LON_ERROR(g_logger) << "connect socket failed, addr=" << addr->toString();
            return nullptr;
        }
        res = new HttpConnection(socket, true, m_buffer_size);
        ++m_size;
    }
    return HttpConnection::Ptr(
        res, std::bind(&HttpConnectionPool::releaseConnection, std::placeholders::_1, this));
}

size_t HttpConnectionPool::size() const { return m_size; }

HttpResult::Ptr HttpConnectionPool::get(const std::string &url, uint64_t timeout_ms,
                                        const http::HttpRequest::MapType &headers,
                                        const std::string &body)
{
    return request(http::HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnectionPool::get(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                                        const http::HttpRequest::MapType &headers,
                                        const std::string &body)
{
    return request(http::HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnectionPool::post(const std::string &url, uint64_t timeout_ms,
                                         const http::HttpRequest::MapType &headers,
                                         const std::string &body)
{
    return request(http::HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnectionPool::post(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                                         const http::HttpRequest::MapType &headers,
                                         const std::string &body)
{
    return request(http::HttpMethod::POST, uri, timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnectionPool::request(http::HttpMethod method, const std::string &url,
                                            uint64_t timeout_ms,
                                            const http::HttpRequest::MapType &headers,
                                            const std::string &body)
{
    auto req = std::make_shared<http::HttpRequest>();
    req->setMethod(method);
    req->setPath(url);
    req->setClose(!m_keepalive);
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
        if (m_vhost.empty())
        {
            req->setHeader("Host", m_host);
        }
        else
        {
            req->setHeader("Host", m_vhost);
        }
    }
    req->setBody(body);

    return request(req, timeout_ms);
}

HttpResult::Ptr HttpConnectionPool::request(http::HttpMethod method, const net::Uri::Ptr &uri,
                                            uint64_t timeout_ms,
                                            const http::HttpRequest::MapType &headers,
                                            const std::string &body)
{
    std::stringstream ss;
    ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
    return request(method, ss.str(), timeout_ms, headers, body);
}

HttpResult::Ptr HttpConnectionPool::request(const http::HttpRequest::Ptr &req, uint64_t timeout_ms)
{
    auto connection = getConnection();
    if (!connection)
    {
        return std::make_shared<HttpResult>(
            (int32_t)HttpResult::Error::CONNECTIONPOOL_GET_CONNECTION_FAILED, nullptr,
            "connectionpool get connection failed, m_host=" + m_host +
                ", m_port=" + util::lexical_cast<std::string>(m_port));
    }
    auto socket = connection->getSocket();
    if (LON_UNLIKELY(!socket))
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::SOCKET_FAILED, nullptr,
                                            "socket create failed");
    }
    socket->setRecvTimeout(timeout_ms);
    auto ret = connection->sendRequest(req);
    if (ret == 0)
    {
        return std::make_shared<HttpResult>((int32_t)HttpResult::Error::SEND_CLOSE_BY_PEER, nullptr,
                                            "send request closed by peer=" +
                                                socket->getPeerAddress()->toString());
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
            "recv response timeout=" + socket->getPeerAddress()->toString() +
                ", timeout ms=" + util::lexical_cast<std::string>(timeout_ms));
    }
    return std::make_shared<HttpResult>((int32_t)HttpResult::Error::OK, response, "");
}

void HttpConnectionPool::releaseConnection(HttpConnection *connection, HttpConnectionPool *pool)
{
    if (!connection->isConnected() ||
        ((connection->getCreateTimeMs() + pool->m_max_alive_time) < util::getCurrentMs()) ||
        (connection->getRequestCount() >= pool->m_max_request_count))
    {
        delete connection;
        connection = nullptr;
        --pool->m_size;
        return;
    }
    ++connection->getRequestCount();
    MutexType::Lock lock(pool->m_mutex);
    pool->m_connections.push_back(connection);
}

} // namespace httpservice
} // namespace lon