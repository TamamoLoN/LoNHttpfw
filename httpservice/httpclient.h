#pragma once

#include "httpservice/httpconnection.h"
#include "httpservice/httpresult.h"
#include "net/uri.h"

namespace lon
{
namespace httpservice
{
class HttpClient
{
  public:
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
};
} // namespace httpservice
} // namespace lon