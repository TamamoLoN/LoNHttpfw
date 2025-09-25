#pragma once

#include "http/httprequest.h"
#include "http/httpresponse.h"

namespace lon
{
namespace httpservice
{
struct HttpResult
{
    using Ptr = std::shared_ptr<HttpResult>;
    enum class Error
    {
        OK = 0,
        INVALID_URL,
        INVALID_HOST,
        SOCKET_FAILED,
        CONNECTION_FAILED,
        SEND_CLOSE_BY_PEER,
        SEND_SOCKET_ERROR,
        RECV_TIMEOUT,
    };
    HttpResult(int32_t result, const http::HttpResponse::Ptr &response, const std::string &error);

    int result;
    http::HttpResponse::Ptr response;
    std::string error;
};
} // namespace httpservice
} // namespace lon