#include "http/httpmethod.h"

namespace lon
{
namespace http
{
HttpMethod HttpMethodConverter::fromString(const std::string &status)
{
    return fromString(status.c_str());
}

std::string HttpMethodConverter::toString(HttpMethod status)
{
    switch (status)
    {
#define XX(code, name, describe)                                                                   \
    case HttpMethod::name:                                                                         \
        return #name;
        HTTP_METHOD_MAP(XX)
#undef XX
    default:
        return "UNKNOWN";
    }
}

HttpMethod HttpMethodConverter::fromString(const char *status)
{
    if (status == nullptr)
    {
        return HttpMethod::UNKNOWN;
    }
#define XX(code, name, describe)                                                                   \
    else if (strcmp(status, #name) == 0) { return HttpMethod::name; }
    HTTP_METHOD_MAP(XX)
#undef XX
    else { return HttpMethod::UNKNOWN; }
}
} // namespace http
} // namespace lon