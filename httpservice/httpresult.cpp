#include "httpservice/httpresult.h"

namespace lon
{
namespace httpservice
{
HttpResult::HttpResult(int32_t result, const http::HttpResponse::Ptr &response,
                       const std::string &error)
    : result(result), response(response), error(error)
{
}
} // namespace httpservice
} // namespace lon