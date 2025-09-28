#pragma once

#include "http/httpparser.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "httpservice/httpresult.h"
#include "net/socketstream.h"
#include "net/uri.h"
#include "thread/mutex.h"

namespace lon
{
namespace httpservice
{
class HttpConnection : public net::SocketStream
{
  public:
    using Ptr = std::shared_ptr<HttpConnection>;
    HttpConnection(const net::Socket::Ptr &socket, bool proxy = true, size_t buffer_size = 4096);
    virtual ~HttpConnection();
    const uint64_t getCreateTimeMs() const;
    uint64_t &getRequestCount();

    http::HttpResponse::Ptr recvResponse();
    ssize_t sendRequest(const http::HttpRequest::Ptr &request);

    static HttpResult::Ptr get(const std::string &url, uint64_t timeout_ms,
                               const http::HttpRequest::MapType &headers = {},
                               const std::string &body                   = "");

    static HttpResult::Ptr get(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                               const http::HttpRequest::MapType &headers = {},
                               const std::string &body                   = "");

    static HttpResult::Ptr post(const std::string &url, uint64_t timeout_ms,
                                const http::HttpRequest::MapType &headers = {},
                                const std::string &body                   = "");

    static HttpResult::Ptr post(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                                const http::HttpRequest::MapType &headers = {},
                                const std::string &body                   = "");

    static HttpResult::Ptr request(http::HttpMethod method, const std::string &url,
                                   uint64_t timeout_ms,
                                   const http::HttpRequest::MapType &headers = {},
                                   const std::string &body                   = "");

    static HttpResult::Ptr request(http::HttpMethod method, const net::Uri::Ptr &uri,
                                   uint64_t timeout_ms,
                                   const http::HttpRequest::MapType &headers = {},
                                   const std::string &body                   = "");

    static HttpResult::Ptr request(const http::HttpRequest::Ptr &req, const net::Uri::Ptr &uri,
                                   uint64_t timeout_ms);

  private:
    size_t m_buffer_size;
    uint64_t m_create_time_ms;
    uint64_t m_request_count;
};

class HttpConnectionPool
{
  public:
    using Ptr       = std::shared_ptr<HttpConnectionPool>;
    using MutexType = thread::Mutex;
    /**
     * @param host 例 127.0.0.1:8080
     * @param vhost
     * @param port 优先级低于host中的端口号，如果host未指定端口号，则使用此端口号。
     * @param max_size
     * @param max_alive_time
     * @param max_request_count
     * @param keepalive
     * @param buffer_size
     */
    HttpConnectionPool(const std::string &host, const std::string &vhost, in_port_t port,
                       uint32_t max_size, uint32_t max_alive_time, uint32_t max_request_count,
                       bool keepalive = true, size_t buffer_size = 4096);
    ~HttpConnectionPool();

    HttpConnection::Ptr getConnection();
    size_t size() const;

    HttpResult::Ptr get(const std::string &url, uint64_t timeout_ms,
                        const http::HttpRequest::MapType &headers = {},
                        const std::string &body                   = "");

    HttpResult::Ptr get(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                        const http::HttpRequest::MapType &headers = {},
                        const std::string &body                   = "");

    HttpResult::Ptr post(const std::string &url, uint64_t timeout_ms,
                         const http::HttpRequest::MapType &headers = {},
                         const std::string &body                   = "");

    HttpResult::Ptr post(const net::Uri::Ptr &uri, uint64_t timeout_ms,
                         const http::HttpRequest::MapType &headers = {},
                         const std::string &body                   = "");

    HttpResult::Ptr request(http::HttpMethod method, const std::string &url, uint64_t timeout_ms,
                            const http::HttpRequest::MapType &headers = {},
                            const std::string &body                   = "");

    HttpResult::Ptr request(http::HttpMethod method, const net::Uri::Ptr &uri, uint64_t timeout_ms,
                            const http::HttpRequest::MapType &headers = {},
                            const std::string &body                   = "");

    HttpResult::Ptr request(const http::HttpRequest::Ptr &req, uint64_t timeout_ms);

  private:
    static void releaseConnection(HttpConnection *connection, HttpConnectionPool *pool);

  private:
    std::string m_host;
    std::string m_vhost;
    in_port_t m_port;
    uint32_t m_max_size;
    uint32_t m_max_alive_time;
    uint32_t m_max_request_count;
    bool m_keepalive;
    size_t m_buffer_size;
    MutexType m_mutex;
    std::list<HttpConnection *> m_connections;
    std::atomic<uint32_t> m_size;
};

} // namespace httpservice
} // namespace lon