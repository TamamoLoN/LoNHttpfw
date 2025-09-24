#pragma once

#include "http/httpparser.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "net/socketstream.h"

namespace lon
{
namespace httpserver
{
class HttpConnection : public net::SocketStream
{
  public:
    using Ptr = std::shared_ptr<HttpConnection>;
    HttpConnection(const net::Socket::Ptr &socket, bool proxy = true, size_t buffer_size = 4096);
    virtual ~HttpConnection();

    http::HttpResponse::Ptr recvResponse();
    ssize_t sendRequest(const http::HttpRequest::Ptr &request);

  private:
    size_t m_buffer_size;
};
} // namespace httpserver
} // namespace lon