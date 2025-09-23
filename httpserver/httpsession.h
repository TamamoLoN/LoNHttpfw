#pragma once

#include "http/httpparser.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "net/socketstream.h"

namespace lon
{
namespace httpserver
{
class HttpSession : public net::SocketStream
{
  public:
    using Ptr = std::shared_ptr<HttpSession>;
    HttpSession(const net::Socket::Ptr &socket, bool proxy = true, size_t buffer_size = 4096);
    virtual ~HttpSession();

    http::HttpRequest::Ptr recvRequest();
    size_t sendResponse(const http::HttpResponse::Ptr &response);

  private:
    size_t m_buffer_size;
};
} // namespace httpserver
} // namespace lon