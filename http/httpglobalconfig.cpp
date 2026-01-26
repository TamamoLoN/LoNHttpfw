#include "http/httpglobalconfig.h"

namespace lon
{
namespace http
{
//全局变量，使其在main函数之前初始化
static auto http_global_conifg = HttpGlobalConfig::Instance();

HttpRequestResponseConfig::HttpRequestResponseConfig(size_t buffer_size, size_t body_size)
    : buffer_size(buffer_size), body_size(body_size)
{
}

bool HttpRequestResponseConfig::operator==(const HttpRequestResponseConfig &other) const
{
    return buffer_size == other.buffer_size && body_size == other.body_size;
}

bool HttpRequestResponseConfig::operator<(const HttpRequestResponseConfig &other) const
{
    return buffer_size < other.buffer_size && body_size < other.body_size;
}

HttpConfig::HttpConfig(HttpRequestResponseConfig request, HttpRequestResponseConfig response)
    : request(request), response(response)
{
}

bool HttpConfig::operator==(const HttpConfig &other) const
{
    return request == other.request && response == other.response;
}

bool HttpConfig::operator<(const HttpConfig &other) const
{
    return request < other.request && response < other.response;
}

HttpGlobalConfig::HttpGlobalConfig()
{
    config_http = config::Config::setData("http", HttpConfig(), "http config");
    config_http->addConfigDataChangeCB([](const HttpConfig &old_data, const HttpConfig &new_data) {
        LON_INFO(LON_LOG_ROOT) << "on config http data changed";
    });
}

HttpGlobalConfig &HttpGlobalConfig::Instance()
{
    static HttpGlobalConfig instance;
    return instance;
}

} // namespace http
} // namespace lon