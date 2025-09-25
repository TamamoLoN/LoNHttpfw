#include "httpserver/httpresult.h"

namespace lon
{
namespace httpserver
{
HttpResult::HttpResult(int32_t result, const http::HttpResponse::Ptr &response,
                       const std::string &error)
    : result(result), response(response), error(error)
{
}
} // namespace httpserver
} // namespace lon