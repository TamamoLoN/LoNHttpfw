#include "http/httpstatus.h"

namespace lon
{
namespace http
{
HttpStatus HttpStatusConverter::fromString(const std::string &status)
{
    return fromString(status.c_str());
}

std::string HttpStatusConverter::toString(HttpStatus status)
{
    switch (status)
    {
#define XX(code, name, describe)                                                                   \
    case HttpStatus::name:                                                                         \
        return #name;
        HTTP_STATUS_MAP(XX)
#undef XX
    default:
        return "UNKNOWN";
    }
}

HttpStatus HttpStatusConverter::fromString(const char *status)
{
    if (status == nullptr)
    {
        return HttpStatus::UNKNOWN;
    }
#define XX(code, name, describe)                                                                   \
    else if (strcmp(status, #name) == 0) { return HttpStatus::name; }
    HTTP_STATUS_MAP(XX)
#undef XX
    else { return HttpStatus::UNKNOWN; }
}

} // namespace http
} // namespace lon